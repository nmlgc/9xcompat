// We want access to the entirety of <windows.h>
#undef _WIN32_WINNT

#include <windows.h>

extern "C" {

// State
// -----

HANDLE StringHeap = nullptr;
// -----

// C library
// ---------

static int compat_wcscmp(const wchar_t *l, const wchar_t *r)
{
	while((*l == *r) && *l && *r) {
		l++;
		r++;
	}
	return ((*l < *r) ? -1 : (*l > *r));
}

static size_t compat_wcslen(const wchar_t *s)
{
	const wchar_t *p = s;
	while(*p) {
		p++;
	}
	return (p - s);
}
// ---------

class ANSI_STRING {
private:
	LPSTR buf = nullptr;

public:
	ANSI_STRING(LPCWSTR str_w)
	{
		const auto default_char = '?';
		const auto str_w_len = (compat_wcslen(str_w) + 1);
		const auto str_a_len = WideCharToMultiByte(
			CP_ACP, 0, str_w, str_w_len, nullptr, 0, &default_char, nullptr
		);
		buf = reinterpret_cast<LPSTR>(
			HeapAlloc(StringHeap, HEAP_GENERATE_EXCEPTIONS, str_a_len)
		);
		WideCharToMultiByte(
			CP_ACP, 0, str_w, str_w_len, buf, str_a_len, &default_char, nullptr
		);
	}

	~ANSI_STRING()
	{
		HeapFree(StringHeap, 0, buf);
	}

	LPSTR data()
	{
		return buf;
	}
};

HANDLE WINAPI compat_FindFirstFileExW(
	LPCWSTR lpFileName,
	FINDEX_INFO_LEVELS fInfoLevelId,
	LPVOID lpFindFileData,
	FINDEX_SEARCH_OPS fSearchOp,
	LPVOID lpSearchFilter,
	DWORD dwAdditionalFlags
)
{
	static decltype(FindFirstFileExW) *orig = nullptr;
	if(!orig) {
		orig = reinterpret_cast<decltype(orig)>(
			GetProcAddress(GetModuleHandleA("kernel32.dll"), "FindFirstFileExW")
		);
	}

	// Windows <7 doesn't support `FindExInfoBasic`.
	return orig(
		lpFileName,
		FindExInfoStandard,
		lpFindFileData,
		fSearchOp,
		lpSearchFilter,
		dwAdditionalFlags
	);
}

DWORD WINAPI compat_FlsAlloc(PFLS_CALLBACK_FUNCTION lpCallback)
{
	return TlsAlloc();
}

BOOL WINAPI compat_FlsFree(DWORD dwFlsIndex)
{
	return TlsFree(dwFlsIndex);
}

PVOID WINAPI compat_FlsGetValue(DWORD dwFlsIndex)
{
	return TlsGetValue(dwFlsIndex);
}

BOOL WINAPI compat_FlsSetValue(DWORD dwFlsIndex, PVOID lpFlsData)
{
	return TlsSetValue(dwFlsIndex, lpFlsData);
}

extern "C++" template <class T> T* cast_to(LPVOID lp, DWORD dwBufferSize)
{
	if(dwBufferSize < sizeof(T)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}
	return reinterpret_cast<T *>(lp);
};

BOOL WINAPI compat_GetFileInformationByHandleEx(
	HANDLE hFile,
	FILE_INFO_BY_HANDLE_CLASS FileInformationClass,
	LPVOID lpfi,
	DWORD lpfi_size
)
{
	BY_HANDLE_FILE_INFORMATION bhfi;
	if(!GetFileInformationByHandle(hFile, &bhfi)) {
		return FALSE;
	}

	switch(FileInformationClass) {
	case FileBasicInfo: {
		auto *ret = cast_to<FILE_BASIC_INFO>(lpfi, lpfi_size);
		if(!ret) {
			return FALSE;
		}
		ret->CreationTime.HighPart = bhfi.ftCreationTime.dwHighDateTime;
		ret->CreationTime.LowPart = bhfi.ftCreationTime.dwLowDateTime;
		ret->LastAccessTime.HighPart = bhfi.ftLastAccessTime.dwHighDateTime;
		ret->LastAccessTime.LowPart = bhfi.ftLastAccessTime.dwLowDateTime;
		ret->LastWriteTime.HighPart = bhfi.ftLastWriteTime.dwHighDateTime;
		ret->LastWriteTime.LowPart = bhfi.ftLastWriteTime.dwLowDateTime;
		ret->ChangeTime.HighPart = bhfi.ftLastWriteTime.dwHighDateTime;
		ret->ChangeTime.LowPart = bhfi.ftLastWriteTime.dwLowDateTime;
		ret->FileAttributes = bhfi.dwFileAttributes;
		break;
	}
	case FileStandardInfo: {
		auto *ret = cast_to<FILE_STANDARD_INFO>(lpfi, lpfi_size);
		if(!ret) {
			return FALSE;
		}
		ret->AllocationSize.LowPart = bhfi.nFileSizeLow;
		ret->AllocationSize.HighPart = bhfi.nFileSizeHigh;
		ret->EndOfFile.LowPart = bhfi.nFileSizeLow;
		ret->EndOfFile.HighPart = bhfi.nFileSizeHigh;
		ret->NumberOfLinks = bhfi.nNumberOfLinks;
		ret->DeletePending = FALSE;
		ret->Directory = (
			(bhfi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
		);
		break;
	}
	case FileAttributeTagInfo: {
		auto *ret = cast_to<FILE_ATTRIBUTE_TAG_INFO>(lpfi, lpfi_size);
		if(!ret) {
			return FALSE;
		}
		ret->FileAttributes = bhfi.dwFileAttributes;
		ret->ReparseTag = 0;
		break;
	}
	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	return true;
}

int WINAPI compat_GetLocaleInfoEx(
	LPCWSTR lpLocaleName, LCTYPE LCType, LPWSTR lpLCData, int cchData
)
{
	// TODO: Implement other LCIDs
	if(compat_wcscmp(lpLocaleName, LOCALE_NAME_SYSTEM_DEFAULT)) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	auto lpLCData_a = reinterpret_cast<LPSTR>(lpLCData);
	int cchData_a = (cchData * sizeof(wchar_t));
	const auto ret = GetLocaleInfoA(
		LOCALE_SYSTEM_DEFAULT, LCType, lpLCData_a, cchData_a
	);
	return (ret / sizeof(wchar_t));
}

BOOL WINAPI compat_InitializeCriticalSectionEx(
	LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount, DWORD Flags
)
{
	return InitializeCriticalSectionAndSpinCount(
		lpCriticalSection, dwSpinCount
	);
}

BOOL WINAPI _DllMainCRTStartup(
	HINSTANCE instance, DWORD reason, LPVOID reserved
)
{
	StringHeap = GetProcessHeap();
	return TRUE;
}

auto ptr_FindFirstFileExW = &compat_FindFirstFileExW;
auto ptr_FlsAlloc = &compat_FlsAlloc;
auto ptr_FlsFree = &compat_FlsFree;
auto ptr_FlsGetValue = &compat_FlsGetValue;
auto ptr_FlsSetValue = &compat_FlsSetValue;
auto ptr_GetFileInformationByHandleEx = &compat_GetFileInformationByHandleEx;
auto ptr_GetLocaleInfoEx = &compat_GetLocaleInfoEx;
auto ptr_InitializeCriticalSectionEx = &compat_InitializeCriticalSectionEx;

} // extern "C"

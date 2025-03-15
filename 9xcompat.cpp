#include <windows.h>

extern "C" {

// Everything here is defined with a different DLL linkage than in the header.
#pragma warning(disable: 4273)

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

DWORD WINAPI FlsAlloc(PFLS_CALLBACK_FUNCTION lpCallback)
{
	return TlsAlloc();
}

BOOL WINAPI FlsFree(DWORD dwFlsIndex)
{
	return TlsFree(dwFlsIndex);
}

PVOID WINAPI FlsGetValue(DWORD dwFlsIndex)
{
	return TlsGetValue(dwFlsIndex);
}

BOOL WINAPI FlsSetValue(DWORD dwFlsIndex, PVOID lpFlsData)
{
	return TlsSetValue(dwFlsIndex, lpFlsData);
}

int WINAPI GetLocaleInfoEx(
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

BOOL WINAPI InitializeCriticalSectionEx(
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

} // extern "C"

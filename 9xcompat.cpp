#include <windows.h>

extern "C" {

// Everything here is defined with a different DLL linkage than in the header.
#pragma warning(disable: 4273)

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
// ---------

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
	return TRUE;
}

} // extern "C"

#include <windows.h>

extern "C" {

// Everything here is defined with a different DLL linkage than in the header.
#pragma warning(disable: 4273)

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

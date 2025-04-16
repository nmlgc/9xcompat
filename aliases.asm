; Aliases for redirecting Windows system DLL imports to function pointers.
; Necessary for statically linking 9xcompat.

extrn _ptr_FindFirstFileExW:near
extrn _ptr_FlsAlloc:near
extrn _ptr_FlsFree:near
extrn _ptr_FlsGetValue:near
extrn _ptr_FlsSetValue:near
extrn _ptr_GetFileInformationByHandleEx:near
extrn _ptr_GetLocaleInfoEx:near
extrn _ptr_InitializeCriticalSectionEx:near

alias <__imp__FindFirstFileExW@24> = <_ptr_FindFirstFileExW>
alias <__imp__FlsAlloc@4> = <_ptr_FlsAlloc>
alias <__imp__FlsFree@4> = <_ptr_FlsFree>
alias <__imp__FlsGetValue@4> = <_ptr_FlsGetValue>
alias <__imp__FlsSetValue@8> = <_ptr_FlsSetValue>
alias <__imp__GetFileInformationByHandleEx@16> = <_ptr_GetFileInformationByHandleEx>
alias <__imp__GetLocaleInfoEx@16> = <_ptr_GetLocaleInfoEx>
alias <__imp__InitializeCriticalSectionEx@12> = <_ptr_InitializeCriticalSectionEx>

end

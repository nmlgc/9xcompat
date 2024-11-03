# Targeting Windows 9x with modern Visual Studio CRTs

## The problem

As of Windows SDK version 10.0.26100.0, Microsoft has removed the last trace of support for Windows 9x from the C runtime by removing the checks in front of Win32 functions introduced in XP or Vista. An example from `winapi_thunks.cpp`:

```diff
extern "C" BOOL WINAPI __acrt_InitializeCriticalSectionEx(
    LPCRITICAL_SECTION const critical_section,
    DWORD              const spin_count,
    DWORD              const flags
    )
{
-    if (auto const initialize_critical_section_ex = try_get_InitializeCriticalSectionEx())
-    {
-        return initialize_critical_section_ex(critical_section, spin_count, flags);
-    }
-
-    return InitializeCriticalSectionAndSpinCount(critical_section, spin_count);
+    return InitializeCriticalSectionEx(critical_section, spin_count, flags);
}
```

These checks are also how you'd handle 9x compatibility in your own code. But the fact that they are no longer present in the C runtime severely hinders your own ability to target 9x. Since the CRT calls this particular function as part of its unconditional initialization code, it's now a fixed part of your binary's DLL imports, causing your program to not even start on 9x anymore. This would also defeat any runtime API hooking techniques, which shouldn't be necessary to begin with if you control the source code.

It might seem that statically linking the CRT (using `/MT` or `/MTd`) would allow you to polyfill these APIs by reimplementing them as part of your statically linked application code, but such a build would still link to the originals from the Windows system DLLs. Taking a look into the CRT library file explains why:

```shell
$ dumpbin /SYMBOLS libucrt.lib | grep InitializeCriticalSectionEx
[…]
212 00000000 UNDEF  notype       External     | __imp__InitializeCriticalSectionEx@12
[…]
```

Basically, Visual Studio's pre-compiled CRT uses the regular Windows headers where all API functions are marked as `declspec(dllimport)`. This adds the `__imp__` prefix to their function names, and these names can only be resolved by a DLL import library. If we don't want to patch, recompile, or otherwise hack the CRT, our polyfilled functions therefore must be part of a DLL.

## Scope

For now, this repo only covers Win32 functions not supported on 9x that the Visual Studio CRT uses behind your back as part of C/C++ standard library implementations. I'm unsure whether this should evolve into a general 9x polyfill library for the entire Win32 API.

No build system. They all suck in some way, and I don't want to pretend that any one of them is "the standard" or "recommended". It's just a single source file anyway.

## Building

We must build with a module definition file to ensure the original undecorated function names despite the `WINAPI` calling convention. The most minimal compilation command line is therefore:

```shell
cl 9xcompat.cpp /link /DLL /OUT:9xcompat.dll /DEF:9xcompat.def kernel32.lib
```

Add more flags as necessary; `/GS-` in particular removes another 1 KiB from the binary.

Linking to the DLL requires a similar trick to make sure that `9xcompat.lib` is processed before `kernel32.lib`. The order is crucial here:

```shell
link […] /NODEFAULTLIB:kernel32.lib 9xcompat.lib kernel32.lib […]
```

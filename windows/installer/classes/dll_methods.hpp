#pragma once

using MB = void (__cdecl*)(const char*, const char*, const char*, const char*, std::string&);
using QS = bool (__cdecl*)(std::string, std::string&);

struct DLL_METHODS {
    MB msgBox;
    QS quietShell;
};

DLL_METHODS loadDllMethods(HMODULE dll) {
    DLL_METHODS methods = {};

    methods.msgBox = reinterpret_cast<MB>(GetProcAddress(dll, "msgBox"));
    methods.quietShell = reinterpret_cast<QS>(GetProcAddress(dll, "quietShell"));

    return methods;
}

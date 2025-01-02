using QS = bool* (__cdecl*)(std::string, std::string&);
using AC = bool* (__cdecl*)(const char*);
using MB = void* (__cdecl*)(const char*, const char*, const char*, const char*, std::string&);

QS const quietShell = reinterpret_cast<QS>(GetProcAddress(DLL, "quietShell"));
AC const access = reinterpret_cast<AC>(GetProcAddress(DLL, "access"));
MB const msgBox = reinterpret_cast<MB>(GetProcAddress(DLL, "msgBox"));

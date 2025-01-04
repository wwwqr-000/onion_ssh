#include <iostream>
#include <windows.h>
#include "classes/resource.h"

int unpackRCResource(int definedVar, std::string name, std::string dropLocation) {
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(definedVar), RT_RCDATA);
    if (!hRes) {
        return 1;
    }
    HGLOBAL hLoadRes = LoadResource(NULL, hRes);
    DWORD resSize = SizeofResource(NULL, hRes);
    void* pResData = LockResource(hLoadRes);
    if (!pResData || resSize == 0) {
        return 2;
    }
    std::ofstream tmpFile(dropLocation + name, std::ios::binary);
    tmpFile.write(reinterpret_cast<const char*>(pResData), resSize);
    tmpFile.close();
    return 0;
}

int main() {
    std::cout << "Test\n";
    return 0;
}

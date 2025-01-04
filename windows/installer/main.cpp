#include <iostream>
#include <windows.h>
#include <fstream>
#include <cctype>//std::isalnum

#include "classes/resource.h"
#include "classes/dll_methods.hpp"

std::string tmpPath = "";
std::string title = "onion ssh";
DLL_METHODS dllMethods;

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

bool isAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(
            &ntAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &adminGroup)) {
        std::cerr << "AllocateAndInitializeSid failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
        std::cerr << "CheckTokenMembership failed. Error: " << GetLastError() << std::endl;
        isAdmin = FALSE;
    }

    if (adminGroup) {
        FreeSid(adminGroup);
    }

    return isAdmin;
}

std::string selectFolder(std::string desc) {
    while (true) {
        std::string buff = "";
        dllMethods.quietShell(("powershell Add-Type -AssemblyName System.Windows.Forms; $dialog = New-Object System.Windows.Forms.FolderBrowserDialog; $dialog.Description = '" + desc + "'; $dialog.ShowNewFolderButton = $true; if ($dialog.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) { $dialog.SelectedPath } else { 'empty' }").c_str(), buff);
        std::string finalBuff = "";
        for (char& c : buff) {
            if (std::isalnum(c) || c == '\\' || c == '/' || c == '_' || c == ' ' || c == '.' || c == '-' || c == '(' || c == ')' || c == '+' || c == '=' || c == ',' || c == ';' || c == '\'' || c == '!' || c == '$' || c == '&' || c == '@' || c == '^' || c == '~' || c == '[' || c == ']' || c == '{' || c == '}' || c == ':') {
                finalBuff += c;
            }
        }
        if (finalBuff != "empty") {
            return finalBuff;
        }
        else {
            dllMethods.msgBox((title + " installer").c_str(), "No folder selected, installation aborted.", "", "e", buff);
            exit(1);
        }
    }
}

void setTmpPath(std::string& path) {
    char buff[MAX_PATH];
    GetTempPath(MAX_PATH, buff);
    path = std::string(buff);
}

void abortInstall(HMODULE DLL, std::string msg = "Installation aborted") {
    std::string buff = "";
    dllMethods.msgBox((title + " installer").c_str(), (msg).c_str(), "", "i", buff);
    FreeLibrary(DLL);
    exit(0);
}

void unzip(std::string from, std::string to) {
    std::string buff = "";
    dllMethods.quietShell(("powershell Expand-Archive -Path '" + from + "' -DestinationPath '" + to + "' -Force").c_str(), buff);
}

void del(std::string file) {
    std::string buff = "";
    dllMethods.quietShell(("del /f \"" + file + "\"").c_str(), buff);
}

int main() {
    setTmpPath(tmpPath);
    if (unpackRCResource(ASSET_WHITEAVOCADO_DLL, "whiteavocado64.dll", tmpPath) != 0) {
        std::cout << "data/whiteavocado64.dll not found. Aborted.";
        return 2;
    }
    HMODULE const DLL = LoadLibraryExA((tmpPath + "whiteavocado64.dll").c_str(), nullptr, 0);
    dllMethods = loadDllMethods(DLL);
    std::string buff = "";

    if (!isAdmin()) {
        abortInstall(DLL, "Installation aborted. You should run the installer as admin.");
    }

    dllMethods.msgBox((title + " installer").c_str(), ("Thanks for downloading Onion ssh!\n\nWould you like to install " + title + "?").c_str(), "yn", "q", buff);
    if (buff != "yes") {
        abortInstall(DLL);
    }
    std::string destenationPath = "";
    while (true) {
        destenationPath = selectFolder("Please select a location where to install " + title);
        dllMethods.msgBox((title + "installer").c_str(), (title + " will get installed at:\n" + destenationPath + "\n\nDo you agree?").c_str(), "yn", "q", buff);
        if (buff != "yes") {
            dllMethods.msgBox((title + " installer").c_str(), "Would you like to choose another location?", "yn", "q", buff);
            if (buff != "yes") {
                abortInstall(DLL);
            }
            continue;
        }
        break;
    }
    std::string packageName = "onion_ssh.zip";
    if (unpackRCResource(ASSET_ONION_SSH, packageName, (destenationPath + "/").c_str()) != 0) {
        abortInstall(DLL, "Installation aborted. Could not extract onion ssh.");
    }
    std::string fullFilePath = destenationPath + "\\" + packageName;
    unzip(fullFilePath, destenationPath);
    del(fullFilePath);
    buff = "";
    dllMethods.quietShell(("powershell.exe -Command \"& { Set-Location 'C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs'; $ws = New-Object -ComObject WScript.Shell; $s = $ws.CreateShortcut($pwd.Path + '\\Onion ssh.lnk'); $s.TargetPath = '" + destenationPath + "\\onion ssh\\onion_ssh.exe" + "'; $s.WorkingDirectory = '" + destenationPath + "\\onion ssh'; $s.Save() }\"").c_str(), buff);
    if (buff != "") {
        dllMethods.msgBox((title + " installer").c_str(), ("Could not create shortcut for " + title).c_str(), "", "w", buff);
        abortInstall(DLL);
    }
    dllMethods.msgBox((title + " installer").c_str(), ("Installed " + title + " successfully!").c_str(), "", "i", buff);
    FreeLibrary(DLL);
    return 0;
}

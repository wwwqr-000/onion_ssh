#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <fstream>

#include "classes/json.hpp"
#include "classes/resource.h"

using json = nlohmann::json;

std::vector<std::thread> threads;
bool intentionalShutdown = false;
HMODULE DLL = LoadLibraryExW(L"data/whiteavocado64.dll", nullptr, 0);
std::string dataFiles[2] = {"data/tor-whiteavocado.exe", "data/config.json"};
json configData;
std::string title = "whiteavocado onion ssh";

#include "classes/dll_methods.hpp"

bool dependenciesExist() {
    for (std::string& f : dataFiles) {
        if (!access(f.c_str())) { return false; }
    }
    return true;
}

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

void cls() {
    system("cls");
}

void wait() {
    system("set /p end=");
}

void torInstance() {
    std::string buff;//Buffer that contains the result of the quietShell instance.
    quietShell(("start " + dataFiles[0]).c_str(), buff);//Instance
    if (intentionalShutdown) { return; }
    msgBox(title.c_str(), "tor instance stopped or already running, ignoring.", "o", "w", buff);
}

void startTor() {
    threads.emplace_back([] { torInstance(); });
}

void killThreads() {
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void stopTor() {
    std::string buff;
    intentionalShutdown = true;
    quietShell("taskkill /IM tor-whiteavocado.exe /F", buff);
}


void setConfig() {
    std::ifstream f(dataFiles[1]);
    configData = json::parse(f);
    f.close();
    std::string buff = "";
    try {
        buff = configData["torService"]["port"];
        buff = configData["torService"]["ip"];
        buff = configData["torService"]["socksVersion"];
        buff = configData["firstUse"];
        buff = "";
    }
    catch (std::exception) {
        msgBox(title.c_str(), (dataFiles[1] + " file is not a valid format or is missing required indexes.").c_str(), "o", "e", buff);
        exit(1);
    }
    if (configData["firstUse"] == "true") {
        configData["firstUse"] = "false";
        std::string jsonStr = configData.dump(4);
        std::ofstream f(dataFiles[1]);
        if (f.is_open()) {
            f << jsonStr;
            f.close();
        }

        msgBox(title.c_str(), "Thanks for downloading the onion ssh tool from whiteavocado!", "o", "i", buff);
    }
}

int main() {
    std::cout << unpackRCResource(IDR_TESTIMG, "test.png", "");
    system(("@echo off && title " + title + " && color a && cls").c_str());
    if (!dependenciesExist()) {
        system("color 4");
        std::cout << "depending file(s) do not exist or no access. Aborted.\n\nPress enter to exit\n";
        wait();
        return 3;
    }
    setConfig();
    startTor();
    std::string connStr = "";
    std::cout << title << "\n\n>ssh ";
    std::getline(std::cin, connStr);
    system(("plink.exe -ssh -proxycmd \"ncat.exe --proxy " + std::string(configData["torService"]["ip"]) + ":" + std::string(configData["torService"]["port"]) + " --proxy-type socks" + std::string(configData["torService"]["socksVersion"]) + " %host %port\" " + connStr).c_str());
    stopTor();
    killThreads();
    return 0;
}

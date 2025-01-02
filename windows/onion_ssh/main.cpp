#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <fstream>

#include "classes/json.hpp"
using json = nlohmann::json;

std::vector<std::thread> threads;
bool intentionalShutdown = false;
HMODULE const DLL = LoadLibraryExW(L"data/whiteavocado64.dll", nullptr, 0);
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
        msgBox(title.c_str(), "Thanks for downloading the onion ssh tool from whiteavocado!\n\nDependencies for this tool:\n* putty (plink)\n* netcat\n\nWould you like to go to the download page of netcat and putty?", "yn", "i", buff);
        if (buff == "yes") {
            system("start https://www.putty.org/ && start https://nmap.org/ncat/");
            buff = "";
            msgBox(title.c_str(), ("Would you like to start " + title + "?").c_str(), "yn", "q", buff);
            if (buff != "yes") { exit(0); }
        }
    }
}

int main() {
    system(("@echo off && title " + title + " && color a && cls").c_str());
    if (!dependenciesExist()) {
        system("color 4");
        std::cout << "depending file(s) do not exist or no access. Aborted.\n\nPress enter to exit\n";
        wait();
        return 2;
    }
    setConfig();
    startTor();
    std::string connStr = "";
    std::cout << title << "\n\n>ssh ";
    std::cin >> connStr;
    system(("plink -ssh -proxycmd \"ncat --proxy " + configData["torService"]["ip"] + ":" + configData["torService"]["port"] + " --proxy-type socks" + configData["torService"]["socksVersion"] + " %host %port\" " + connStr).c_str());
    stopTor();
    killThreads();
    return 0;
}

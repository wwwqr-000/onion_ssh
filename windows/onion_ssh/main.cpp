#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>

std::vector<std::thread> threads;
bool intentionalShutdown = false;

HMODULE const DLL = LoadLibraryExW(L"whiteavocado64.dll", nullptr, 0);

std::string dataFiles[4] = {"tor-whiteavocado.exe", "nc-whiteavocado.exe", "config.json", "plink-whiteavocado.exe"};

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
    quietShell("start tor-whiteavocado.exe", buff);//Instance
    if (intentionalShutdown) { return; }
    msgBox("whiteavocado onion ssh", "tor instance stopped or already running, ignoring.", "o", "w", buff);
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

int main() {
    system("@echo off && title whiteavocado onion ssh && color a && cls");
    if (!dependenciesExist()) {
        system("color 4");
        std::cout << "depending file(s) do not exist or no access. Aborted.\n\nPress enter to exit\n";
        wait();
        return 1;
    }
    startTor();
    std::string connStr = "";
    std::cout << "whiteavocado onion ssh\n\n>ssh ";
    std::cin >> connStr;
    system((dataFiles[3] + " -ssh -proxycmd \"ncat --proxy 127.0.0.1:9050 --proxy-type socks5 %host %port\" " + connStr).c_str());
    stopTor();
    killThreads();

    return 0;
}

#pragma once

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <tchar.h>
#include <sstream>
#include <algorithm>
#include <winternl.h>
#include <Powrprof.h>
#include <Shellapi.h>
#include <fstream>
#include <experimental/filesystem>

#pragma comment(lib, "Powrprof.lib")
#pragma comment(lib, "ntdll.lib")

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation
} MEMORY_INFORMATION_CLASS;
extern "C" NTSTATUS ZwQueryVirtualMemory(HANDLE, void*, MEMORY_INFORMATION_CLASS, void*, SIZE_T, SIZE_T*);
extern "C" NTSTATUS ZwReadVirtualMemory(HANDLE, void*, void*, SIZE_T, SIZE_T*);
extern "C" NTSTATUS ZwWriteVirtualMemory(HANDLE, void*, void*, SIZE_T, SIZE_T*);


class Foudasi {
public:

    void terminate_process(DWORD pid) {
        if (HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, pid)) {
            BOOL result = TerminateProcess(hProcess, 1);
            CloseHandle(hProcess);
        }
    }

    DWORD get_service_pid(const std::string& service_name) {
        SC_HANDLE scm = OpenSCManager(nullptr, nullptr, NULL);
        SC_HANDLE sc = OpenService(scm, service_name.c_str(), SERVICE_QUERY_STATUS);

        SERVICE_STATUS_PROCESS ssp = { 0 };
        DWORD bytes_needed = 0;
        QueryServiceStatusEx(sc, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&ssp), sizeof(ssp),
            &bytes_needed);

        CloseServiceHandle(sc);
        CloseServiceHandle(scm);

        return ssp.dwProcessId;
    }

    double cleanedstrings = 0;

    std::string getexepath() {
        char path[MAX_PATH];
        ::GetModuleFileName(0, path, MAX_PATH);
        return path;
    }

    std::string getexename() {
        std::string path = getexepath();
        std::string exename = path.substr(path.rfind("\\") + 1);
        return exename;
    }

    std::vector<std::string> exevector() {
        std::vector<std::string> returnme;
        std::string exepath = getexepath();
        returnme.push_back(exepath);

        std::string exename = getexename();
        returnme.push_back(exename);

        return returnme;
    }

    

    DWORD get_pid(const std::string& process_name) {
        PROCESSENTRY32 process_info;
        process_info.dwSize = sizeof(process_info);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (snapshot == INVALID_HANDLE_VALUE)
            return 0;

        Process32First(snapshot, &process_info);
        if (!process_name.compare(process_info.szExeFile)) {
            CloseHandle(snapshot);
            return process_info.th32ProcessID;
        }

        while (Process32Next(snapshot, &process_info)) {
            if (!process_name.compare(process_info.szExeFile)) {
                CloseHandle(snapshot);
                return process_info.th32ProcessID;
            }
        }

        CloseHandle(snapshot);

        return 0;
    }

    std::vector<std::string> hostvector() {
        std::vector<std::string> returnme = {
                "keyauth.win" };
        return returnme;
    }

    void cleanstrings(DWORD pid, std::vector<std::string> findvector, bool ispca = false) {

        if (HANDLE processhandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid)) {
            for (size_t i = 0; i < findvector.size(); i++) {
                std::string lmao = findvector[i];
                for (const char x : lmao) {
                    if (!isprint(x)) {
                        findvector.erase(findvector.begin() + i);
                        break;
                    }
                }
            }

            if (pid != GetCurrentProcessId()) {
                MEMORY_BASIC_INFORMATION memory;
                INT64 address = 0;
                while (NT_SUCCESS(ZwQueryVirtualMemory(processhandle, (LPVOID)address, MemoryBasicInformation, &memory,
                    sizeof(MEMORY_BASIC_INFORMATION), nullptr))) {
                    if (memory.State == MEM_COMMIT && (
                        (memory.Protect == PAGE_EXECUTE_READWRITE) |
                        (memory.Protect == PAGE_READWRITE) |
                        (memory.Protect == PAGE_EXECUTE_WRITECOPY) |
                        (memory.Protect == PAGE_WRITECOPY))) {

                            {
                                std::vector<char> buffer(memory.RegionSize);
                                if (NT_SUCCESS(
                                    ZwReadVirtualMemory(processhandle, (LPVOID)address, &buffer[0], memory.RegionSize,
                                        nullptr))) {
                                    for (std::string x : findvector) {
                                        INT64 lenght = x.length();
                                        INT64 size = static_cast<INT64>(memory.RegionSize);
                                        for (INT64 i = 0; i <= size; i++) {
                                            INT64 j;
                                            for (j = 0; j < lenght; j++)
                                                if (buffer[i + j] != x[j])
                                                    break;
                                            if (j == lenght) {

                                                if (ispca) {

                                                    std::string current_pca;
                                                    size_t current_pca_start = address + i;
                                                    size_t location = current_pca_start;
                                                    size_t new_pca_size = 0;

                                                    for (;;) {

                                                        std::vector<char> char_a(1);
                                                        if (ReadProcessMemory(processhandle, (LPVOID)location, &char_a[0],
                                                            (sizeof(char) * 1), 0)) {

                                                            current_pca.push_back(char_a[0]);

                                                        }

                                                        std::vector<char> char_b(5);
                                                        if (ReadProcessMemory(processhandle, (LPVOID)location, &char_b[0],
                                                            (sizeof(char) * 5), 0)) {

                                                            size_t counter = 0;
                                                            for (const char x : char_b) {
                                                                if (x <= 126 && x >= 32) {}
                                                                else { counter++; }
                                                            }
                                                            if (counter == 5) { break; }
                                                            else { new_pca_size += 2; }

                                                        }

                                                        location++;

                                                    }

                                                    size_t addition = 0;
                                                    std::string new_pca;
                                                    std::istringstream iss(current_pca);
                                                    for (std::string line; std::getline(iss, line);) {

                                                        if (line.find(getexename()) != std::string::npos) {

                                                            addition += line.length() + 2;

                                                        }
                                                        else { new_pca += (line); }

                                                    }

                                                    char nullthat = 0;
                                                    for (size_t m = 0; m < current_pca.length(); m++) {
                                                        WriteProcessMemory(processhandle, (LPVOID)(current_pca_start + m),
                                                            &nullthat, 1, 0);
                                                    }

                                                    for (size_t m = 0; m < new_pca.length(); m++) {
                                                        WriteProcessMemory(processhandle,
                                                            (LPVOID)(current_pca_start + addition + m),
                                                            &new_pca[m], 1, 0);
                                                    }

                                                    break;

                                                }
                                                else {
                                                    std::string newstring;
                                                    for (size_t y = 0; y < lenght; y++) { newstring.push_back(0); }
                                                    WriteProcessMemory(processhandle, (LPVOID)(address + i), &newstring,
                                                        lenght, 0);
                                                }

                                                cleanedstrings++;
                                            }
                                        }
                                    }
                                }
                            }

                            {
                                if (!ispca) {
                                    std::vector<WCHAR> buffer(memory.RegionSize);
                                    if (NT_SUCCESS(
                                        ZwReadVirtualMemory(processhandle, (LPVOID)address, &buffer[0], memory.RegionSize,
                                            nullptr))) {
                                        for (std::string x : findvector) {
                                            INT64 lenght = x.length();
                                            INT64 size = static_cast<INT64>(memory.RegionSize);
                                            for (INT64 i = 0; i <= size; i++) {
                                                INT64 j;
                                                for (j = 0; j < lenght; j++)
                                                    if (buffer[i + j] != x[j])
                                                        break;
                                                if (j == lenght) {
                                                    cleanedstrings++;
                                                    WCHAR writeme = 0;
                                                    INT64 x = i;
                                                    INT64 counter = 0;
                                                    for (;;) {
                                                        WriteProcessMemory(processhandle, (LPVOID)(address + (x * (INT64)2)),
                                                            &writeme, (sizeof(WCHAR)), 0);
                                                        x++;
                                                        counter++;
                                                        if (counter >= lenght) { break; }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                    }
                    address += memory.RegionSize;
                }
            }
        }

    }

    std::vector<std::string> pcaclarinet() {
        std::vector<std::string> x;
        std::string one = "TRACE,0000,0000,PcaClient,MonitorProcess," + getexepath() + ",Time,0";
        std::string two = R"(\n)";
        x.push_back(one);
        x.push_back(two);
        return x;
    }

    
    
    
    void disable_hibernation()
    {
        LPCSTR lpOperation = "runas"; // para executar com privilégios de administrador
        LPCSTR lpFile = "powercfg.exe";
        LPCSTR lpParameters = "/hibernate off";
        LPCSTR lpDirectory = NULL;
        int nShowCmd = SW_HIDE; // para ocultar a janela do Prompt de Comando

        HINSTANCE result = ShellExecuteA(NULL, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);

        if ((int)result <= 32) {
            // Se ocorrer um erro, você pode lidar com ele aqui.
            // Por exemplo, exibir uma mensagem de erro usando a função GetLastError() e FormatMessage().
        }
    }

    void cleanprefetch() {
        std::string prefetchstring = "\\Windows\\prefetch\\";
        std::string findmeprefetch = getexename();
        std::transform(findmeprefetch.begin(), findmeprefetch.end(), findmeprefetch.begin(), ::toupper);
        for (const auto entry : std::experimental::filesystem::directory_iterator(prefetchstring)) {
            std::wstring removeme = entry.path();
            std::string removemestring(removeme.begin(), removeme.end());
            if (strstr(removemestring.c_str(), findmeprefetch.c_str())) {
                remove(removemestring.c_str());
            }

            if (strstr(removemestring.c_str(), "WMIC")) {
                remove(removemestring.c_str());
            }
        }
    }

    void disable_usb_logging()
    {
        HKEY hkey;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\USBSTOR\\Parameters", 0, KEY_WRITE, &hkey) == ERROR_SUCCESS)
        {
            DWORD value = 1;
            if (RegSetValueEx(hkey, "DisableDeviceLog", 0, REG_DWORD, (const BYTE*)&value, sizeof(value)) == ERROR_SUCCESS)
            {
                RegSetValueEx(hkey, "EnableLog", 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
            }
        }
        RegCloseKey(hkey);
    }

    void removeUSB() {
        std::string path = "C:\\Windows\\System32\\Config\\USBSTOR";
        remove(path.c_str());
    }

    void deleteRegistryUSBKey() {
        HKEY key;
        LPCSTR subkey = "SYSTEM\\CurrentControlSet\\Enum\\USBSTOR";
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {
            RegDeleteTree(key, NULL);
            RegCloseKey(key);
        }
    }

    void deleteSetupApi() {
        std::ofstream file("C:\\Windows\\inf\\Setupapi.log");
        if (file.is_open()) {
            file.close();
        }
    }

    void DisableTimestamp() {
        const char* keyPath = "SYSTEM\\CurrentControlSet\\Control\\WMI\\Autologger\\Circular Kernel Context Logger";
        const char* valueName = "EnableLog";
        DWORD valueData = 0;

        RegSetKeyValueA(HKEY_LOCAL_MACHINE, keyPath, valueName, REG_DWORD, &valueData, sizeof(valueData));
    }

    void disableWindowLog() {
        HKEY hKey;
        LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\EventLog", 0, KEY_SET_VALUE, &hKey);

        if (result == ERROR_SUCCESS) {
            DWORD value = 4;
            result = RegSetValueEx(hKey, "Start", 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));

            if (result == ERROR_SUCCESS) {
                RegCloseKey(hKey);
            }

        }
    }

    void destructAll() {
        cleanprefetch();
        disable_hibernation();
        removeUSB();
        deleteRegistryUSBKey();
        DisableTimestamp();
        disableWindowLog();

        cleanstrings(get_pid("smartscreen.exe"), exevector());
        cleanstrings(get_pid("smartscreen.exe"), hostvector());
        terminate_process(get_pid("smartscreen.exe"));
        cleanstrings(get_pid("SearchIndexer.exe"), exevector());
        cleanstrings(get_pid("ctfmon.exe"), exevector());
        cleanstrings(get_pid("lsass.exe"), hostvector());
        cleanstrings(get_service_pid("PcaSvc"), exevector());
        cleanstrings(get_service_pid("DiagTrack"), exevector());
        cleanstrings(get_service_pid("DPS"), exevector());
        cleanstrings(get_service_pid("Dnscache"), hostvector());
        cleanstrings(get_pid("explorer.exe"), pcaclarinet(), true);
    }
};

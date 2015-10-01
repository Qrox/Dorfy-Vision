#ifdef BUILD_EXE

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string.h>

using namespace std;

#ifdef DEBUG
string dllPath = "C:\\Users\\Qrox\\Eclipse\\Dorfy Vision\\DllDebug\\Dorfy Vision.dll";
#else
string dllPath = "Dorfy Vision.dll";
#endif

char constexpr dorfProcessName[] = "Dwarf Fortress.exe";

HANDLE locateDorfs() {
    cout << "looking for game process..." << endl;

    static PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        cout << "failed to create toolhelp32 snapshot!" << endl;
        return nullptr;
    }
    if (Process32First(snapshot, &entry) == TRUE) {
        do {
            if (strcmp(entry.szExeFile, dorfProcessName) == 0) {
                HANDLE ret = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, entry.th32ProcessID);
                if (ret) {
                    cout << "found " << dorfProcessName << ", trying to inject dll..." << endl;
                    return ret;
                } else {
                    cout << "failed to open " << dorfProcessName << endl;
                }
            }
        } while (Process32Next(snapshot, &entry) == TRUE);
    }
    CloseHandle(snapshot);
    return nullptr;
}

int main(void) {
    HANDLE dorf = locateDorfs();
    if (!dorf) {
        cout << "cannot find dwarf fortress.exe. run this application after dwarf fortress is running" << endl;
        return 0;
    }
    // Dorfs found. Time for !!SCIENCE!!

    HMODULE localDll = nullptr;
    PVOID loadLibraryParameter = nullptr, remoteParam = nullptr;

    do {
        localDll = LoadLibrary(dllPath.data());
        if (!localDll) {
            cout << "failed to load Dll \"" << dllPath << "\". error code: " << GetLastError() << endl;
            break;
        }
        LPTHREAD_START_ROUTINE localEntry = (LPTHREAD_START_ROUTINE) GetProcAddress(localDll, "DllEntry");
        if (!localEntry) {
            cout << "function DllEntry doesn't exist in \"" << dllPath << "\"" << endl;
            break;
        }
        DWORD entryOffset = (DWORD) localEntry - (DWORD) localDll;

        HMODULE kernel32 = GetModuleHandle("kernel32");
        if (!kernel32) {
            cout << "cannot find kernel32.dll" << endl;
            break;
        }

        unsigned int dirLen = GetCurrentDirectory(0, nullptr);
        if (dirLen == 0) {
            cout << "failed to get the working directory" << endl;
            break;
        }
        char * pcurrentDir = new char[dirLen];
        if (pcurrentDir == nullptr) {
            cout << "failed to allocate memory to get the working directory" << endl;
            break;
        }
        if (GetCurrentDirectory(dirLen, pcurrentDir) != dirLen - 1) {
            cout << "failed to get the working directory" << endl;
            break;
        }
        string currentDir = string(pcurrentDir);
        delete [] pcurrentDir;
        if (currentDir.back() != '\\' && currentDir.back() != '/') currentDir.push_back('\\');

#ifdef DEBUG
        loadLibraryParameter = VirtualAllocEx(dorf, 0, dllPath.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!loadLibraryParameter) {
            cout << "failed to allocate parameter memory for kernel32::LoadLibraryA" << endl;
            break;
        }
        if (!WriteProcessMemory(dorf, loadLibraryParameter, dllPath.data(), dllPath.length(), 0)) {
            cout << "failed to write to parameter memory for kernel32::LoadLibraryA" << endl;
            break;
        }
#else
        string fullPath = currentDir + dllPath;
        loadLibraryParameter = VirtualAllocEx(dorf, 0, fullPath.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!loadLibraryParameter) {
            cout << "failed to allocate parameter memory for kernel32::LoadLibraryA" << endl;
            break;
        }
        if (!WriteProcessMemory(dorf, loadLibraryParameter, fullPath.data(), fullPath.length(), 0)) {
            cout << "failed to write to parameter memory for kernel32::LoadLibraryA" << endl;
            break;
        }
#endif

        HANDLE thread = CreateRemoteThread(dorf, 0, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(kernel32, "LoadLibraryA"), loadLibraryParameter, 0, 0);
        if (!thread) {
            cout << "failed to call kernel32::LoadLibraryA" << endl;
            break;
        }
        WaitForSingleObject(thread, INFINITE);
        HMODULE dllModule = nullptr;
        if (!GetExitCodeThread(thread, (DWORD *) &dllModule)) {
            cout << "failed to get return value from kernel32::LoadLibraryA" << endl;
            CloseHandle(thread);
            break;
        }
        CloseHandle(thread);
        if (!dllModule) {
            cout << "failed to load remote library" << endl;
        } else {
            cout << "dll injected" << endl;
        }
        if (!VirtualFreeEx(dorf, loadLibraryParameter, 0, MEM_RELEASE)) {
            cout << "failed to free parameter memory for kernel32::LoadLibraryA" << endl;
        }
        loadLibraryParameter = nullptr;
        if (!dllModule) {
            break;
        }

        struct {
            DWORD processId;
            char * currentDir;
        } param;
        remoteParam = VirtualAllocEx(dorf, 0, sizeof(param) + currentDir.length(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (remoteParam == nullptr) {
            cout << "failed to allocate memory for DLL parameters" << endl;
            break;
        }
        param.processId = GetCurrentProcessId();
        param.currentDir = (char *) remoteParam + sizeof(param);
        if (!WriteProcessMemory(dorf, remoteParam, &param, sizeof(param), 0) ||
            !WriteProcessMemory(dorf, param.currentDir, currentDir.data(), currentDir.length(), 0)) {
            cout << "failed to write parameters into DLL" << endl;
            break;
        }

        thread = CreateRemoteThread(dorf, 0, 0, (LPTHREAD_START_ROUTINE) ((DWORD) dllModule + entryOffset), remoteParam, 0, 0);
        if (!thread) {
            cout << "failed to start injected dll" << endl;
            break;
        }
        WaitForSingleObject(thread, INFINITE);

        if (!VirtualFreeEx(dorf, remoteParam, 0, MEM_RELEASE)) {
            cout << "failed to free memory for DLL parameters" << endl;
        }
        remoteParam = nullptr;

        DWORD status = -1;
        if (!GetExitCodeThread(thread, &status)) {
            cout << "failed to get return status from the dll" << endl;
        } else {
            cout << "injected dll exited with status " << status << endl;
        }
        CloseHandle(thread);

        thread = CreateRemoteThread(dorf, 0, 0, (LPTHREAD_START_ROUTINE) GetProcAddress(kernel32, "FreeLibrary"), dllModule, 0, 0);
        if (!thread) {
            cout << "failed to free injected dll" << endl;
        } else {
            WaitForSingleObject(thread, INFINITE);
            CloseHandle(thread);
        }
    } while (false);
    if (localDll) {
        FreeLibrary(localDll);
        localDll = nullptr;
    }
    if (loadLibraryParameter) {
        if (!VirtualFreeEx(dorf, loadLibraryParameter, 0, MEM_RELEASE)) {
            cout << "failed to free parameter memory for kernel32::LoadLibraryA" << endl;
        }
        loadLibraryParameter = nullptr;
    }
    if (remoteParam) {
        if (!VirtualFreeEx(dorf, remoteParam, 0, MEM_RELEASE)) {
            cout << "failed to free memory for DLL parameters" << endl;
        }
        remoteParam = nullptr;
    }
    return 0;
}

#endif

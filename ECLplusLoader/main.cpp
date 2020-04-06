/*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce this list of conditions and the following disclaimer
*    in the documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
* OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Windows.h>
#include <TlHelp32.h>

#define DLLNAME L"ECLPLUS.dll"
#define GAMENAME L"th17.exe"

#define MSG_ERROR(msg) (MessageBoxW(NULL, TEXT(msg), L"Error", MB_OK)) 

HANDLE GetRunningHandle() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(entry);

    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        return NULL;
    }
    do {
        if (CompareStringOrdinal(entry.szExeFile, -1, GAMENAME, -1, TRUE) == CSTR_EQUAL) {
            CloseHandle(snapshot);
            return OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return NULL;
}

HANDLE CreateNewHandle() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    BOOL success = CreateProcessW(GAMENAME, NULL, NULL, NULL, FALSE,
        0, NULL, NULL, &si, &pi);
    if (!success) {
        return NULL;
    }

    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
}

int main() {
    BOOL appliedToRunning = TRUE;
    HANDLE handle = GetRunningHandle();
    if (!handle) {
        appliedToRunning = FALSE;
        handle = CreateNewHandle();
    }

    if (handle == NULL) {
        MSG_ERROR("Game not found. The loader should either be in the same folder as th17.exe, or th17 should be started before running the loader (and it must be named th17.exe).");
        return 1;
    }

    LPVOID addr = VirtualAllocEx(handle, NULL, sizeof(DLLNAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (addr == NULL) {
        MSG_ERROR("DLL injection failed: unable to allocate memory with VirtualAllocEx. Try running the loader as administrator");
        return 1;
    }

    WriteProcessMemory(handle, addr, DLLNAME, sizeof(DLLNAME), NULL);
    
    LPVOID lib = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if (lib == NULL) {
        MSG_ERROR("DLL injection failed: unable to locate LoadLibraryW (this should literally never happen)");
        return 1;
    }

    HANDLE thread = CreateRemoteThread(handle, NULL, NULL, (LPTHREAD_START_ROUTINE)lib, addr, NULL, NULL);

    if (thread == NULL) {
        MSG_ERROR("DLL injection failed: unable to create a remote thread. Try running the loader as administrator");
        return 1;
    }
    
    if (WaitForSingleObject(thread, 5000) == WAIT_TIMEOUT) {
        MSG_ERROR("DLL injection (probably) failed: remote thread did not return after 5 seconds");
        return 1;
    }

    HANDLE ret;
    GetExitCodeThread(thread, (LPDWORD)&ret);
    CloseHandle(handle);

    if (ret == NULL) {
        MSG_ERROR(
            "DLL injection failed, this could be for a variety of reasons:\n"
            "- ECLPLUS.dll is not in the same folder as th17.exe\n"
            "- the game version is not the correct one (must be 1.00b)\n"
            "- using the Steam version and trying to launch it with the loader. Instead of that, run the game first, then the loader\n"
            "- there could be some other reasons that I'm not aware of. Try running the loader as administrator, maybe?"
        );
        return 1;
    } else if (appliedToRunning) {
        MessageBoxW(NULL, L"Applied ECLplus to a running instance of th17.exe", L"Success", MB_OK);
    }

    return 0;
}

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

#define DLLNAME L"ECLPLUS.dll"

int main() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    BOOL success = CreateProcessW(L"th17.exe", NULL, NULL, NULL, FALSE,
                                  0, NULL, NULL, &si, &pi);
    if (!success) {
        MessageBoxW(NULL, L"Failed to start th17.exe", L"Error", MB_OK);
        return 1;
    }

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
    if (handle == NULL) {
        MessageBoxW(NULL, L"An error occured when trying to open th17.exe process", L"Error", MB_OK);
        return 2;
    }

    LPVOID addr = VirtualAllocEx(handle, NULL, sizeof(DLLNAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(handle, addr, DLLNAME, sizeof(DLLNAME), NULL);
    
    LPVOID lib = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    CreateRemoteThread(handle, NULL, NULL, (LPTHREAD_START_ROUTINE)lib, addr, NULL, NULL);
    
    CloseHandle(handle);
    return 0;
}

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
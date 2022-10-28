#include "amcache.h"

void removeAmcache(wchar_t* executableName)
{
    // Deletes the associated Amcache entry
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    wchar_t cmdLineArgs[120];
    wcsncpy_s(cmdLineArgs, L"/c REG LOAD HKLM\\AM C:\\Windows\\appcompat\\Programs\\Amcache.hve", 119);
    int createProcessStatus = CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", cmdLineArgs, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 10);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    HKEY hKey;
    RegOpenKeyW(HKEY_LOCAL_MACHINE, L"AM\\Root\\InventoryApplicationFile", &hKey);

    // Amcache max length of exe name in keys is 16 and all in lowercase
    wchar_t executableNameTruncated[17];
    wcsncpy_s(executableNameTruncated, 17, executableName, 16);
    toLowerString(executableNameTruncated);

    wprintf(L"[DEBUG] Amcache key extracted: %s\n", executableNameTruncated);

    // Enumerate through all subkeys. If the truncated executable name is a part of the key name, delete it
    wchar_t keyName[256];
    int index = 0;
    DWORD cchName = 256;
    do
    {
        keyName[0] = '\0';
        RegEnumKeyExW(hKey, index, keyName, &cchName, NULL, NULL, NULL, NULL);

        // No more keys to enumerate
        if (keyName[0] == '\0')
            break;

        wprintf(L"[DEBUG] Enumerating key: %s\n", keyName);

        wchar_t* substrPtr = wcsstr(keyName, executableNameTruncated);

        // Found the key to delete!
        if (substrPtr != NULL)
        {
			wprintf(L"[DEBUG] Deleting key: %s\n", keyName);
            //int err = RegDeleteKeyW(hKey, keyName);
            //wprintf(L"[DEBUG] Error for deleting key in amcache: %d\n", err);

            //if (err == ERROR_SUCCESS)
                //wprintf(L"[DEBUG] Successfully deleted key: %s\n", keyName);
        }

    } while (TRUE);
}
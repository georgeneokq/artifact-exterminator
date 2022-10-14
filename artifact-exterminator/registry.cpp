#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "registry.h"
#include "utils.h"

/*
 * Element size limits (including null terminator)
   Key: 256
   Value name: 16384
   Data: 1MB (1000000B)
*/

LPCWSTR getKeyNameByHandle(HKEY hKey)
{
    if (hKey == HKEY_LOCAL_MACHINE) {
        return L"HKEY_LOCAL_MACHINE";
    }
    else if (hKey == HKEY_CURRENT_USER) {
        return L"HKEY_CURRENT_USER";
    }
    else if (hKey == HKEY_USERS) {
        return L"HKEY_USERS";
    }
    else if (hKey == HKEY_CURRENT_CONFIG) {
        return L"HKEY_CURRENT_CONFIG";
    }
    else {
        return L"Unknown";
    }
}

void backupRegistry(LPCWSTR backupPath)
{
    WCHAR tempPath[MAX_PATH + 1] = { 0 };
    GetTempPathW(MAX_PATH + 1, tempPath);

    HKEY rootHKeys[4] = {
        HKEY_CURRENT_CONFIG,
        HKEY_CURRENT_USER,
        HKEY_LOCAL_MACHINE,
        HKEY_USERS
    };

    HANDLE ProcessToken;

    // Set privilege for backing up registry
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &ProcessToken)) {
        SetPrivilege(ProcessToken, SE_BACKUP_NAME, TRUE);

        for (int i = 0; i < 4; i++) {
            HKEY rootKeyHandle = rootHKeys[i];
            
            // Backup subkeys of each root key one by one.
            // Saving a master hive directly is not allowed.
            int enumSubkeysIndex = 0;
            while (true) {
                WCHAR subkeyName[KEY_LIMIT] = { 0 };
                DWORD bufferSize = KEY_LIMIT;
                HKEY subkeyHandle;

                RegEnumKeyExW(rootKeyHandle, enumSubkeysIndex, subkeyName, &bufferSize, NULL, NULL, NULL, NULL);
                if (*subkeyName == NULL) break;
                RegOpenKeyExW(rootKeyHandle, subkeyName, NULL, KEY_ALL_ACCESS, &subkeyHandle);

                // Form the file name to save the hive to
                WCHAR filename[50 + KEY_LIMIT] = { 0 };
                wsprintf(filename, L"%s\\%s %s.hiv", backupPath, getKeyNameByHandle(rootKeyHandle), subkeyName);

                // Save the hive
                int saveKeyErr = RegSaveKeyExW(subkeyHandle, filename, NULL, REG_STANDARD_FORMAT);
                wprintf(L"Error writing to %s: %d\n", filename, saveKeyErr);

                enumSubkeysIndex++;
            }
        }
    }
}

/*
 * DEPRECATED.
 * Function that enumerates all subkeys
 */
void recursiveEnumKeys(HKEY rootKey, LPCWSTR subKey)
{
    int enumKeyIndex = 0;
    WCHAR lpName[KEY_LIMIT] = { 0 };
    DWORD bufferSize = KEY_LIMIT;
    while (true) {
        // Initialize lpName to NULL at the start of every enumeration
        lpName[0] = NULL;

        // Open the key passed in via function parameters
        HKEY hKey;
        RegOpenKeyEx(rootKey, subKey, NULL, KEY_ALL_ACCESS, &hKey);

        // For each subkey
        RegEnumKeyExW(hKey, enumKeyIndex, lpName, &bufferSize, NULL, NULL, NULL, NULL);

        // If no subkey name was returned, end the loop
        if (*lpName == 0) {
            break;
        }
        // If a subkey was found, call this recursive function to enumerate subkeys of the subkey
        else {
            recursiveEnumKeys(hKey, lpName);
        }

        wprintf(L"%s\n", lpName);
        enumKeyIndex++;
    }
}

void backupRegistrySimple(LPCWSTR backupPath)
{
    const DWORD cRootKeys = 4;
    HKEY rootHKeys[cRootKeys] = {
        HKEY_CURRENT_CONFIG,
        HKEY_CURRENT_USER,
        HKEY_LOCAL_MACHINE,
        HKEY_USERS
    };
    PROCESS_INFORMATION processInfos[cRootKeys];
    HANDLE processHandles[cRootKeys];
    
    for (int i = 0; i < 4; i++)
    {
        LPCWSTR keyName = getKeyNameByHandle(rootHKeys[i]);
        
        // Form the file name for backup
        WCHAR fullBackupPath[MAX_PATH * 2 + 1];
        wsprintf(fullBackupPath, L"%s\\%s.reg", backupPath, keyName);
        
        // Form the reg export command
        WCHAR cmdLineArgs[256];
        wsprintf(cmdLineArgs, L"/c reg export %s %s /y", keyName, fullBackupPath);

        // Run the command by spawning a new cmd process for each root key
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        wprintf(L"Running command `C:\\Windows\\System32\\cmd.exe %s`\n", cmdLineArgs);
        int createProcessStatus = CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", cmdLineArgs, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);

        // Keep track of handles for teardown after all processes complete
        processInfos[i] = pi;
        processHandles[i] = pi.hProcess;
        if (createProcessStatus == 0)
        {
            wprintf(L"CreateProcess error: %d\n", GetLastError());
        }
    }

    // Close handles
    WaitForMultipleObjects(cRootKeys, processHandles, TRUE, INFINITE);
    for (int i = 0; i < cRootKeys; i++)
    {
        CloseHandle(processInfos[i].hThread);
        CloseHandle(processInfos[i].hProcess);
    }
}

void restoreRegistry(LPCWSTR backupPath)
{
    // Use reg import on the reg files in the specified path
    WCHAR fullBackupPath[MAX_PATH * 2 + 1];
    wsprintf(fullBackupPath, L"%s\\*", backupPath);
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFileW(fullBackupPath, &data);
    
    // Assume a maximum of 6 processes
    PROCESS_INFORMATION processInfos[6] = { 0 };
    HANDLE processHandles[6] = { 0 };

    int fileIndex = 0;
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0)
                continue;
            // Form the file name for backup
            WCHAR fullBackupPath[MAX_PATH * 2 + 1];
            wsprintf(fullBackupPath, L"%s\\%s", backupPath, data.cFileName);
            WCHAR cmdLine[256];

            STARTUPINFOW si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(pi));
            wsprintf(cmdLine, L"/c reg import %s", fullBackupPath);
            CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", cmdLine, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
            processInfos[fileIndex] = pi;
            processHandles[fileIndex] = pi.hProcess;
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
    wprintf(L"Restoring registry...\n");
    WaitForMultipleObjects(fileIndex + 1, processHandles, TRUE, INFINITE);
    for (int i = 0; i < fileIndex; i++)
    {
        CloseHandle(processInfos[i].hThread);
        CloseHandle(processInfos[i].hProcess);
    }
}
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
    if (hKey == HKEY_LOCAL_MACHINE)
        return L"HKEY_LOCAL_MACHINE";
    else if (hKey == HKEY_CURRENT_USER)
        return L"HKEY_CURRENT_USER";
    else if (hKey == HKEY_USERS)
        return L"HKEY_USERS";
    else if (hKey == HKEY_CURRENT_CONFIG)
        return L"HKEY_CURRENT_CONFIG";
    else if (hKey == HKEY_CLASSES_ROOT)
        return L"HKEY_CLASSES_ROOT";
    else
        return L"Unknown";
}

HKEY getHandleByKeyName(wchar_t* keyName)
{
    if (wcscmp(keyName, L"HKEY_LOCAL_MACHINE") == 0 || wcscmp(keyName, L"HKLM") == 0)
        return HKEY_LOCAL_MACHINE;
    else if (wcscmp(keyName, L"HKEY_CURRENT_USER") == 0 || wcscmp(keyName, L"HKCU") == 0)
        return HKEY_CURRENT_USER;
    else if (wcscmp(keyName, L"HKEY_USERS") == 0 || wcscmp(keyName, L"HKU") == 0)
        return HKEY_USERS;
    else if (wcscmp(keyName, L"HKEY_CURRENT_CONFIG") == 0 || wcscmp(keyName, L"HKLM") == 0)
        return HKEY_CURRENT_CONFIG;
    else if (wcscmp(keyName, L"HKEY_CLASSES_ROOT") == 0 || wcscmp(keyName, L"HKCR") == 0)
        return HKEY_CLASSES_ROOT;
    else
        return NULL;
}

void backupRegistry(LPCWSTR backupPath)
{
    const DWORD cRootKeys = 5;
    HKEY rootHKeys[cRootKeys] = {
        HKEY_CURRENT_CONFIG,
        HKEY_CURRENT_USER,
        HKEY_LOCAL_MACHINE,
        HKEY_USERS,
        HKEY_CLASSES_ROOT
    };
    PROCESS_INFORMATION processInfos[cRootKeys];
    HANDLE processHandles[cRootKeys];
    
    for (int i = 0; i < cRootKeys; i++)
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

void deleteRegistryKeys(int numKeys, wchar_t** keys)
{
    for (int i = 0; i < numKeys; i++)
    {
        wchar_t* key = keys[i];
        wchar_t* rootKeyName;
        // Extract root key from string
        wchar_t* nextToken;

        // Root key
        wchar_t* separatorPtr = wcschr(key, L'\\');
        int separatorPos = separatorPtr - key;
        rootKeyName = (wchar_t*)malloc(sizeof(wchar_t) * (separatorPos + 1));
        wcsncpy_s(rootKeyName, separatorPos + 1, key, separatorPos);
        rootKeyName[separatorPos] = '\0';

        // Subkey
        wchar_t* subkey = separatorPtr + 1;

        wprintf(L"[DEBUG] Root key: %s, Subkey: %s\n", rootKeyName, subkey);

        RegDeleteKeyW(getHandleByKeyName(rootKeyName), subkey);

        free(rootKeyName);
    }
}

void deleteRegistryValues(int numValues, RegValue* values)
{
    for (int i = 0; i < numValues; i++)
    {
        RegValue regValue = values[i];
        wchar_t* rootKeyName;
        // Extract root key from string
        wchar_t* nextToken;

        // Root key
        wchar_t* separatorPtr = wcschr(regValue.keyName, L'\\');
        int separatorPos = separatorPtr - regValue.keyName;
        rootKeyName = (wchar_t*)malloc(sizeof(wchar_t) * (separatorPos + 1));
        wcsncpy_s(rootKeyName, separatorPos + 1, regValue.keyName, separatorPos);
        rootKeyName[separatorPos] = '\0';

        // Subkey
        wchar_t* subkey = separatorPtr + 1;

        wprintf(L"[DEBUG] Root key: %s, Subkey: %s, Value: %s\n", rootKeyName, subkey, regValue.valueName);

        RegDeleteKeyValueW(getHandleByKeyName(rootKeyName), subkey, regValue.valueName);

        free(rootKeyName);
    }
}
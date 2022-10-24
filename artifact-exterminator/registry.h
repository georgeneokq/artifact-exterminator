#pragma once
#include <Windows.h>
#include "registry.h"

#define KEY_LIMIT 256
#define VALUE_LIMIT 16384
#define DATA_LIMIT 1000000
#define KEY_VALUE_SEPARATOR L':'

struct RegValue
{
    wchar_t* keyName;
    wchar_t* valueName;
};

LPCWSTR getKeyNameByHandle(HKEY hKey);
HKEY getHandleByKeyName(wchar_t* keyName);
void backupRegistry(LPCWSTR backupPath);
void restoreRegistry(LPCWSTR backupPath);
void restoreRegistry(LPCWSTR backupPath, BOOL deleteBackupFiles);
void deleteRegistryValues(int numValues, RegValue* values);
void deleteRegistryKeys(int numKeys, wchar_t** keys);

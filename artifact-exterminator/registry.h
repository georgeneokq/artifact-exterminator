#pragma once
#include <Windows.h>
#include "registry.h"

#define KEY_LIMIT 256
#define VALUE_LIMIT 16384
#define DATA_LIMIT 1000000

void backupRegistry(LPCWSTR backupPath);
void restoreRegistry(LPCWSTR backupPath);
LPCWSTR getKeyNameByHandle(HKEY hKey);

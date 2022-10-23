#pragma once
#include <Windows.h>
#define REGISTRY_BACKUP_FOLDER_NAME L"regbackup"

void getRegistryBackupFolderPath(DWORD nBufferLength, LPWSTR lpBuffer);
void scheduleShimcacheTask(wchar_t* executableNames);

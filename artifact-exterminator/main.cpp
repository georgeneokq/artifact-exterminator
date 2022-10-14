#include <iostream>
#include <Windows.h>
#include "main.h"
#include "registry.h"


int main()
{
    WCHAR registryBackupFolderPath[MAX_PATH + 1] = { 0 };
    getRegistryBackupFolderPath(MAX_PATH + 1, registryBackupFolderPath);

    // Create folder to make registry backup
    CreateDirectoryW(registryBackupFolderPath, NULL);

    // Perform more operations here...
}

void getRegistryBackupFolderPath(DWORD nBufferLength, LPWSTR lpBuffer)
{
    GetTempPathW(nBufferLength, lpBuffer);
    wcscat_s(lpBuffer, nBufferLength, REGISTRY_BACKUP_FOLDER_NAME);
}
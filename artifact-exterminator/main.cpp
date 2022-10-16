#include <iostream>
#include <Windows.h>
#include "main.h"
#include "registry.h"
#include "shimcache.h"
#include "utils.h"


int main(int argc, char* argv[])
{
    WCHAR registryBackupFolderPath[MAX_PATH + 1] = { 0 };
    getRegistryBackupFolderPath(MAX_PATH + 1, registryBackupFolderPath);

    // Create folder to make registry backup
    CreateDirectoryW(registryBackupFolderPath, NULL);

    // Parse command line arguments
    char executableFilePath[MAX_PATH + 1] = { 0 };
    char registryKeysToRemove[1024] = { 0 };
    char registryValuesToRemove[1024] = { 0 };
    char runOnlyShimcacheRemoval[2] = { 0 };
    char additionalExecutableNames[1024] = { 0 };

    /* 
     * Argument list. Values should come after their flags, separated by spaces.
     * e.g. -f C:\Windows\System32\executable.exe
     * -f File path of executable
     * -k Registry keys to remove, comma-separated
     * -v Registry values to remove, comma-separated. Value name should come after the key, separated by colon
     *    e.g. HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\AppCompatCache:AppCompatCache
     *    The root key can be specified by either its full name or by its shorthand
     *    i.e. HKEY_LOCAL_MACHINE HKLM
     * -a Additional file paths to clean up
     * -s Only run shimcache removal function. The value of this option is not relevant, but is still required.
     *    e.g. artifact-exterminator.exe -f C:\Windows\System32\executable.exe -s 1
     */
    
	getCommandLineValue(argc, argv, "-a", additionalExecutableNames, 1024);

    // Run only the shimcache removal function.
    if (getCommandLineValue(argc, argv, "-s", runOnlyShimcacheRemoval, 2))
    {
	    // TODO: Perform shimcache cleanup and quit the program
    }

 
    if (!getCommandLineValue(argc, argv, "-f", executableFilePath, MAX_PATH + 1))
    {
        printf("Argument -f is required.");
        return 1;
    }

    getCommandLineValue(argc, argv, "-k", registryKeysToRemove, 1024);
    getCommandLineValue(argc, argv, "-v", registryValuesToRemove, 1024);

    printf("[DEBUG]\n-f: %s\n-k: %s\n-v: %s\n-s: %s\n-a: %s\n",
        executableFilePath,
        registryKeysToRemove,
        registryValuesToRemove,
        runOnlyShimcacheRemoval,
        additionalExecutableNames);
    
    // TODO: Schedule task to perform shimcache cleanup upon system reboot

    // TODO: Backup registry

    // TODO: Run executable specified by -f argument

    // TODO: Restore registry

    return 0;
}

void getRegistryBackupFolderPath(DWORD nBufferLength, LPWSTR lpBuffer)
{
    GetTempPathW(nBufferLength, lpBuffer);
    wcscat_s(lpBuffer, nBufferLength, REGISTRY_BACKUP_FOLDER_NAME);
}
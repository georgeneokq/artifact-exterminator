/*
 * This program should be ran with administrative privileges.
 * Credentials should also be provided through command line parameters -u and -p,
 * for the shimcache removal function to work.
 * At the moment, none of the functionalities will work without administrative privileges.
 */

#include <iostream>
#include <Windows.h>
#include "main.h"
#include "registry.h"
#include "shimcache.h"
#include "utils.h"


int wmain(int argc, wchar_t* argv[])
{
    WCHAR registryBackupFolderPath[MAX_PATH] = { 0 };
    getRegistryBackupFolderPath(MAX_PATH, registryBackupFolderPath);

    // Create folder to make registry backup
    CreateDirectoryW(registryBackupFolderPath, NULL);

    // Parse command line arguments
    wchar_t executableFilePath[MAX_PATH] = { 0 };
    wchar_t registryKeysToRemove[1024] = { 0 };
    wchar_t registryValuesToRemove[1024] = { 0 };
    wchar_t runOnlyShimcacheRemoval[2] = { 0 };
    wchar_t additionalExecutableNames[1024] = { 0 };
    wchar_t adminUsername[50] = { 0 };
    wchar_t adminPassword[100] = { 0 };

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

    getCommandLineValue(argc, argv, L"-u", adminUsername, 50);
    getCommandLineValue(argc, argv, L"-p", adminPassword, 100);
    
	getCommandLineValue(argc, argv, L"-a", additionalExecutableNames, 1024);

    // Run only the shimcache removal function.
    if (getCommandLineValue(argc, argv, L"-s", runOnlyShimcacheRemoval, 2))
    {
	    // TODO: Perform shimcache cleanup and quit the program
    }

    if (!getCommandLineValue(argc, argv, L"-f", executableFilePath, MAX_PATH))
    {
        printf("Argument -f is required.");
        return 1;
    }

    getCommandLineValue(argc, argv, L"-k", registryKeysToRemove, 1024);
    getCommandLineValue(argc, argv, L"-v", registryValuesToRemove, 1024);

    wprintf(L"[DEBUG]\n-f: %s\n-k: %s\n-v: %s\n-s: %s\n-a: %s\n",
        executableFilePath,
        registryKeysToRemove,
        registryValuesToRemove,
        runOnlyShimcacheRemoval,
        additionalExecutableNames);

    // Extract executable name from -f parameter
	wchar_t* executableFileName = wcsrchr(executableFilePath, L'\\');
    if (executableFileName == NULL)
        executableFileName = executableFilePath;
    else
        // Skip the delimiter
        executableFileName = executableFileName + 1;

    // Combine with with -a parameter, comma-separated
    wchar_t executableNames[1024 + MAX_PATH];
    if (*additionalExecutableNames != NULL)
    {
        swprintf_s(executableNames, L"%s,%s", additionalExecutableNames, executableFileName);
    }
    else
    {
        wcsncpy_s(executableNames, executableFileName, wcslen(executableFileName));
    }
    wprintf(L"[DEBUG] Executables to remove from shimcache: %s\n", executableNames);
    
    // TODO: Schedule task to perform shimcache cleanup upon system reboot
    if (!scheduleShimcacheTask(executableNames))
        wprintf(L"[ERROR] Unable to schedule task to remove shimcache entries\n");

    // TODO: Backup registry

    // TODO: Run executable specified by -f argument

    // TODO: Restore registry

    return 0;
}

/*
 * Schedule a task using schtasks
 * 
 * @param char* executableNames  Comma-separated list of executable names, as passed in by -a parameter
 * @return BOOL  If successfully created task, return TRUE
 */
BOOL scheduleShimcacheTask(wchar_t* executableNames)
{
    wchar_t filePath[MAX_PATH];
    GetModuleFileNameW(NULL, filePath, MAX_PATH);

    // A task name that looks legitimate
    const wchar_t* taskName = L"MicrosoftEdgeUpdateTaskMachineCA";

    wchar_t command[512];
    _snwprintf_s(command, 512, L"/c SCHTASKS /Create /F /RU SYSTEM /SC ONSTART /TN %s /TR \"%s -s 1 -a %s\"", taskName, filePath, executableNames);
    wprintf(L"[DEBUG] Task creation command:\n%s\n", command);

	STARTUPINFOW si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
    CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", command, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, 10000);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}

void getRegistryBackupFolderPath(DWORD nBufferLength, LPWSTR lpBuffer)
{
    GetTempPathW(nBufferLength, lpBuffer);
    wcscat_s(lpBuffer, nBufferLength, REGISTRY_BACKUP_FOLDER_NAME);
}

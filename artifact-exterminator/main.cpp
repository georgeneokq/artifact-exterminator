/*
 * This program should be ran with administrative privileges.
 * Credentials should also be provided through command line parameters -u and -p,
 * for the shimcache removal function to work.
 * 
 * Functionality will be limited without administrative privileges.
 * For example, registry backup and restoration will not work for HKLM hive.
 */

#include <iostream>
#include <Windows.h>
#include "main.h"
#include "registry.h"
#include "shimcache.h"
#include "prefetch.h"
#include "utils.h"
#include "killswitch.h"


/* 
 * Argument list. Values should come after their flags, separated by spaces.
 * e.g. -f C:\Windows\System32\executable.exe
 * -f File path of executable
 * --args Arguments for specified executable
 * --killswitch-ip IPv4 address of kill switch socket
 * --killswitch-port Port of remote socket
 * --killswitch-poll Interval for polling, in seconds. Defaults to once every 10 seconds.
 * -k Registry keys to remove, comma-separated.
 *    If any part of the argument contains a space, it should be wrapped in quotes.
 *    The root key can be specified by either its full name or by its shorthand
 *    i.e. HKEY_LOCAL_MACHINE HKLM
 * -v Registry values to remove, comma-separated. Value name should come after the key, separated by a colon.
	  If any part of the argument contains a space, it should be wrapped in quotes.
 *    e.g. HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\AppCompatCache\AppCompatCache
 * -a Additional file paths to clean up
 * -s Only run shimcache removal function. The value of this option is not relevant, but is still required.
 *    e.g. artifact-exterminator.exe -f C:\Windows\System32\executable.exe -s 1
 *    The user should almost never have to use this argument. This is mainly used when scheduling a task to clear the shimcache.
 * --features Specify features to run, comma-separated. If this argument is not provided, all features are ran by default.
 *            Possible values:
 *            registry
 *            shimcache
 *            prefetch
 */
int wmain(int argc, wchar_t* argv[])
{
    // Print a warning message if administrator privileges not detected.
    if (!IsElevated())
        wprintf(L"[WARNING] Administrator privileges not detected, running with limited functionality.\n");

    WCHAR registryBackupFolderPath[MAX_PATH] = { 0 };
    getRegistryBackupFolderPath(MAX_PATH, registryBackupFolderPath);

    // Create folder to make registry backup
    CreateDirectoryW(registryBackupFolderPath, NULL);


    // Parse command line arguments
    wchar_t executableFilePath[MAX_PATH] = { 0 };
    wchar_t commandArgs[512] = { 0 };
    wchar_t features[120] = { 0 };
    wchar_t killSwitchIP[20] = { 0 };
    wchar_t killSwitchPort[6] = { 0 };
    wchar_t killSwitchPollIntervalStr[10] = { 0 };
    wchar_t registryKeysToRemoveStr[1024] = { 0 };
    wchar_t registryValuesToRemoveStr[1024] = { 0 };
    wchar_t runOnlyShimcacheRemoval[2] = { 0 };
    wchar_t additionalExecutableNames[1024] = { 0 };

    getCommandLineValue(argc, argv, L"-a", additionalExecutableNames, 1024);
    getCommandLineValue(argc, argv, L"-s", runOnlyShimcacheRemoval, 2);
    getCommandLineValue(argc, argv, L"-f", executableFilePath, MAX_PATH);
    getCommandLineValue(argc, argv, L"--args", commandArgs, 512);

    /*
     * Features to enable, comma-separated.
     * Current possible options:
     * registry
     * shimcache
     * prefetch
     */
    getCommandLineValue(argc, argv, L"--features", features, 120);

    // Kill switch args
    getCommandLineValue(argc, argv, L"--killswitch-ip", killSwitchIP, 20);
    getCommandLineValue(argc, argv, L"--killswitch-port", killSwitchPort, 6);
    getCommandLineValue(argc, argv, L"--killswitch-poll", killSwitchPollIntervalStr, 10);

    // XOR, if one is given but the other isn't
    if (!(*killSwitchIP != NULL) != !(*killSwitchPort != NULL))
    {
        wprintf(L"[WARNING] Either --killswitch-ip or --killswitch-port was provided, but the other was not. Kill switch polling not enabled.\n");
    }

    // Registry deletion args
    getCommandLineValue(argc, argv, L"-k", registryKeysToRemoveStr, 1024);
    getCommandLineValue(argc, argv, L"-v", registryValuesToRemoveStr, 1024);

    wprintf(L"[DEBUG]\n-f: %s\n--args: %s\n--features: %s\n-k: %s\n-v: %s\n-s: %s\n-a: %s\n--killswitch-ip: %s\n--killswitch-port: %s\n--killswitch-poll: %s\n",
        executableFilePath,
        commandArgs,
        features,
        registryKeysToRemoveStr,
        registryValuesToRemoveStr,
        runOnlyShimcacheRemoval,
        additionalExecutableNames,
        killSwitchIP,
        killSwitchPort,
        killSwitchPollIntervalStr);

    // Parse --features argument, comma-separated. All features enabled by default if argument not provided.
    BOOL registryFeatureEnabled = FALSE;
    BOOL shimcacheFeatureEnabled = FALSE;
    BOOL prefetchFeatureEnabled = FALSE;

    // Enable specified features
    if (*features != NULL)
    {
        wchar_t* nextToken;
        wchar_t* token = wcstok_s(features, L",", &nextToken);
        while (token)
        {
            wprintf(L"[DEBUG] Enabling module \"%s\"...\n", token);
            if (wcscmp(token, L"registry") == 0)
                registryFeatureEnabled = TRUE;
            else if (wcscmp(token, L"shimcache") == 0)
                shimcacheFeatureEnabled = TRUE;
            else if (wcscmp(token, L"prefetch") == 0)
                prefetchFeatureEnabled = TRUE;
            else
                wprintf(L"Unknown module \"%s\".\n", token);
            token = wcstok_s(NULL, L",", &nextToken);
        }
    }
    // --features argument not provided, enable all features
    else
    {
        wprintf(L"[DEBUG] --features argument not provided, enabling all features.\n");
        registryFeatureEnabled = TRUE;
        shimcacheFeatureEnabled = TRUE;
        prefetchFeatureEnabled = TRUE;
    }

    // Convert registry deletion args (-k and -v) from comma-separated values to actual array
    wchar_t* registryKeysToRemove[50];
    int numKeys = 0;
    RegValue registryValuesToRemove[50];
    int numValues = 0;

    // Convert -k argument to array of strings
    if (*registryKeysToRemoveStr != NULL)
    {
        wchar_t* nextToken;
        wchar_t* token = wcstok_s(registryKeysToRemoveStr, L",", &nextToken);
        while (token)
        {
            size_t tokenLen = wcslen(token);
            registryKeysToRemove[numKeys] = (wchar_t*)malloc(sizeof(wchar_t) * (tokenLen + 1));
            wcsncpy_s(registryKeysToRemove[numKeys], tokenLen + 1, token, tokenLen);
            numKeys++;
            token = wcstok_s(NULL, L",", &nextToken);
        }
    }

    // Convert -v argument to array of RegValue structs
    if (*registryValuesToRemoveStr != NULL)
    {
        wchar_t* nextToken;
        wchar_t* token = wcstok_s(registryValuesToRemoveStr, L",", &nextToken);
        while (token)
        {
            // Key and value comes in pairs, separted by character specified in registry.h
            wchar_t* separatorPtr = wcsrchr(token, KEY_VALUE_SEPARATOR);
            int separatorPos = separatorPtr - token;
            wchar_t* key = (wchar_t*)malloc(sizeof(wchar_t) * (separatorPos + 1));
            wcsncpy_s(key, separatorPos + 1, token, separatorPos);
            key[separatorPos] = '\0';
            wchar_t* value = separatorPtr + 1;

            RegValue regValue = { key, value };
            registryValuesToRemove[numValues] = regValue;
            numValues++;
            token = wcstok_s(NULL, L",", &nextToken);
        }
    }

    // Extract executable name from -f argument
    wchar_t* executableFileName = wcsrchr(executableFilePath, L'\\');
    if (executableFileName == NULL)
        executableFileName = executableFilePath;
    else
        // Skip the delimiter
        executableFileName = executableFileName + 1;

    // Combine -f argument with -a argument, comma-separated
    wchar_t executableNames[1024 + MAX_PATH];
    if (*additionalExecutableNames != NULL)
    {
        swprintf_s(executableNames, L"%s,%s", additionalExecutableNames, executableFileName);
    }
    else
    {
        wcsncpy_s(executableNames, executableFileName, wcslen(executableFileName));
    }
    wprintf(L"[DEBUG] Executables to remove traces of: %s\n", executableNames);
    
    // FEAT: Schedule task to perform shimcache cleanup upon system reboot
    if(shimcacheFeatureEnabled)
		scheduleShimcacheTask(executableNames);

    // FEAT: Backup registry
    if(registryFeatureEnabled)
		backupRegistry(registryBackupFolderPath);

    // FEAT: Run executable specified by -f argument
    if (*executableFilePath != NULL)
    {
		STARTUPINFOW si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));
		wchar_t command[512 + MAX_PATH];
		wsprintf(command, L"/c %s %s", executableFilePath, commandArgs);
		CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", command, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

        // After closing the executable, artifacts like prefetch files may take some time to be created.
        Sleep(2000);
    }

    // FEAT: Poll for kill switch before performing any cleanup
    if (*killSwitchIP != NULL && *killSwitchPort != NULL)
    {
		int killSwitchPollInterval = 10;
		int port = _wtoi(killSwitchPort);
		if (*killSwitchPollIntervalStr != NULL)
			killSwitchPollInterval = _wtoi(killSwitchPollIntervalStr);

		pollKillSwitch(killSwitchIP, port, killSwitchPollInterval);
    }

    // FEAT: Restore registry to previous state
    if (registryFeatureEnabled)
    {
		restoreRegistry(registryBackupFolderPath);
		wprintf(L"Deleting registry values...\n");
		deleteRegistryValues(numValues, registryValuesToRemove);

		wprintf(L"Deleting registry keys...\n");
		deleteRegistryKeys(numKeys, registryKeysToRemove);
    }

    // Remove shimcache records of specified executable names
    if (shimcacheFeatureEnabled && runOnlyShimcacheRemoval)
    {
        // Split additionalExecutableNames by comma.
        // For each executable name, remove shimcache record.
        wchar_t* nextToken;
        wchar_t* token = wcstok_s(additionalExecutableNames, L",", &nextToken);
        while (token)
        {
            removeShimcache(token);
            token = wcstok_s(NULL, L",", &nextToken);
        }
    }

    // FEAT: Clear prefetch records of specified executable names
    if(prefetchFeatureEnabled)
    {
        wchar_t* nextToken;
        wchar_t* token = wcstok_s(executableNames, L",", &nextToken);

        while (token)
        {
            wprintf(L"[DEBUG] Removing prefetch file for %s\n", token);
            if (!clearPrefetch(token))
                wprintf(L"[DEBUG] Unable to find prefetch file for %s\n", token);
            token = wcstok_s(NULL, L",", &nextToken);
        }
    }

    // Cleanup: Free pointers
    for (int i = 0; i < numKeys; i++)
        free(registryKeysToRemove[i]);
    
    // Only keyName is created using malloc
    for (int i = 0; i < numValues; i++)
        free(registryValuesToRemove[i].keyName);

    return 0;
}

/*
 * Schedule a task using schtasks
 * 
 * @param char* executableNames  Comma-separated list of executable names, as passed in by -a parameter
 * @return BOOL  If successfully created task, return TRUE
 */
void scheduleShimcacheTask(wchar_t* executableNames)
{
    wchar_t filePath[MAX_PATH];
    GetModuleFileNameW(NULL, filePath, MAX_PATH);

    // A task name that looks legitimate
    const wchar_t* taskName = L"MicrosoftEdgeUpdateTaskMachineCA";

    wchar_t command[512];
    _snwprintf_s(
        command,
        512,
        L"/c SCHTASKS /Create /F /RU SYSTEM /SC ONSTART /TN %s /TR \"%s -s 1 --features shimcache -a %s\"",
        taskName,
        filePath,
        executableNames);
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
}

void getRegistryBackupFolderPath(DWORD nBufferLength, LPWSTR lpBuffer)
{
    GetTempPathW(nBufferLength, lpBuffer);
    wcscat_s(lpBuffer, nBufferLength, REGISTRY_BACKUP_FOLDER_NAME);
}

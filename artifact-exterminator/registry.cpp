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

void backupRegistry()
{
	WCHAR tempPath[MAX_PATH + 1] = { 0 };
	GetTempPathW(MAX_PATH + 1, tempPath);

	// Create folder to make registry backup
	WCHAR registryBackupPath[MAX_PATH + 1 + 30] = { 0 };
	wsprintf(registryBackupPath, L"%sbackup", tempPath);
	CreateDirectoryW(registryBackupPath, NULL);

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
				wsprintf(filename, L"%sbackup\\%s %s.hiv", tempPath, getKeyNameByHandle(rootKeyHandle), subkeyName);

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
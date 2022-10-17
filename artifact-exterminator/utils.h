#pragma once
#include <Windows.h>
#include "utils.h"

BOOL SetPrivilege(
    HANDLE hToken,
    LPCTSTR lpszPrivilege,
    BOOL bEnablePrivilege
);

BOOL getCommandLineValue(int argc, wchar_t* argv[], const wchar_t* option, wchar_t* buf, size_t bufSize);

#pragma once
#include <Windows.h>
#include "utils.h"

BOOL SetPrivilege(
    HANDLE hToken,
    LPCTSTR lpszPrivilege,
    BOOL bEnablePrivilege
);

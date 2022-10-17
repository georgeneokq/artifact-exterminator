#pragma once
#pragma comment(lib, "Ws2_32.lib")
#include <Windows.h>

void pollKillSwitch(wchar_t* wstrSocketIP, int port, int pollInterval);

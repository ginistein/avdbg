
#pragma once

#include "MainFrm.h"

DWORD WINAPI DebugThreadProc(LPVOID lpParameter);
BOOL GetFileNameFromHandle(HANDLE hFile, TCHAR* pszFilename);
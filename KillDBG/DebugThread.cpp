
#include "stdafx.h"
#include "DebugThread.h"


DWORD WINAPI DebugThreadProc(LPVOID lpParameter)
{
	if (!lpParameter)
	{
		return 1;
	}

	CMainFrame*	pMainFrm = (CMainFrame*)lpParameter;
	LPVOID targetRVA;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {0};
	GetStartupInfo(&si);
	DebugSetProcessKillOnExit(TRUE);
	CreateProcess(pMainFrm->m_strExePath, NULL, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi); //DEBUG_PROCESS?
	DEBUG_EVENT DBEvent;
	while (!pMainFrm->m_bExit)
	{
		WaitForDebugEvent(&DBEvent,INFINITE);

		switch(DBEvent.dwDebugEventCode)	//we just cleanup here, but this method may have performance issue
		{
		case CREATE_PROCESS_DEBUG_EVENT:	//创建被调试进程
			CloseHandle(DBEvent.u.CreateProcessInfo.hFile);
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			break;

		case LOAD_DLL_DEBUG_EVENT:		//加载DLL
			{
				CloseHandle(DBEvent.u.LoadDll.hFile);
			}
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			break;

		case RIP_EVENT:
			break;

		case EXCEPTION_DEBUG_EVENT:
			break;
		}
		ContinueDebugEvent(DBEvent.dwProcessId, DBEvent.dwThreadId, DBG_CONTINUE);
	}

	return 0;
}

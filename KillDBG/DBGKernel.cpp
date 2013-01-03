#include "StdAfx.h"
#include "MainFrm.h"
#include "DBGKernel.h"


CDBGKernel::CDBGKernel(void)
	:m_bAttach(FALSE),
	m_bExit(FALSE),
	m_dwPID(0),
// 	m_hDbgProcess(NULL),
	m_hDebugThread(NULL),
	m_pMainFrm(NULL)
{
	m_pMainFrm = (CMainFrame*)AfxGetMainWnd();
}


CDBGKernel::~CDBGKernel(void)
{
// 	if (m_hDbgProcess && m_hDbgProcess != INVALID_HANDLE_VALUE)
// 	{
// 		CloseHandle(m_hDbgProcess);
// 		m_hDbgProcess = NULL;
// 	}
	if (m_hDebugThread && m_hDebugThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDebugThread);
		m_hDebugThread = NULL;
	}
}

BOOL CDBGKernel::CreateDbgProcess( CString strExePath, CString strParam, CString strRunDir )
{
	if (  strExePath == _T("")
		||strRunDir == _T(""))
	{
		return FALSE;
	}
	m_strExePath = strExePath;
	m_strParam = strParam;
	m_strRunDir = strRunDir;
	m_bAttach = FALSE;

	DWORD dwThreadID;
	m_hDebugThread = CreateThread(NULL,0,&DebugThreadProc,this,NULL,&dwThreadID);

	return m_hDebugThread != INVALID_HANDLE_VALUE;
}

BOOL CDBGKernel::AttachDbgProcess( DWORD dwPID )
{
	if (!dwPID)
	{
		return FALSE;
	}
	m_dwPID = dwPID;
	m_bAttach = TRUE;

	DWORD dwThreadID;
	m_hDebugThread = CreateThread(NULL,0,&DebugThreadProc,this,NULL,&dwThreadID);

	return m_hDebugThread != INVALID_HANDLE_VALUE;
}

BOOL CDBGKernel::GetFileNameFromHandle( HANDLE hFile, TCHAR* pszFilename )
{
	BOOL bSuccess = FALSE;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	// Create a file mapping object.
	HANDLE hFileMap = CreateFileMapping(hFile, 
		NULL, 
		PAGE_READONLY,
		0, 
		dwFileSizeLo,
		NULL);

	if (hFileMap) 
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem) 
		{
			if (GetMappedFileName (GetCurrentProcess(), 
				pMem, 
				pszFilename,
				MAX_PATH)) 
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[512];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(512-1, szTemp)) 
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do 
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, 512))
						{
							UINT uNameLen = _tcslen(szName);

							if (uNameLen < MAX_PATH) 
							{
								bFound = _tcsnicmp(pszFilename, szName, 
									uNameLen) == 0;

								if (bFound) 
								{
									// Reconstruct pszFilename using szTemp
									// Replace device path with DOS path
									TCHAR szTempFile[MAX_PATH];
									_stprintf(szTempFile,
										TEXT("%s%s"),
										szDrive,
										pszFilename+uNameLen);
									_tcsncpy(pszFilename, szTempFile, MAX_PATH);
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		} 

		CloseHandle(hFileMap);
	}
	return(bSuccess);
}

DWORD WINAPI CDBGKernel::DebugThreadProc( LPVOID lpParameter )
{
	CDBGKernel*	pDBGKrnl = (CDBGKernel*)lpParameter;
	if (pDBGKrnl->m_bAttach)
	{
		DebugSetProcessKillOnExit(FALSE);
		if (!DebugActiveProcess(pDBGKrnl->m_dwPID))
		{
			pDBGKrnl->m_pMainFrm->MessageBox(_T("附加指定进程失败！！"),NULL,MB_OK | MB_ICONERROR);
			return 1;
		}
	} 
	else
	{
		STARTUPINFO si = {0};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi = {0};
		GetStartupInfo(&si);
		DebugSetProcessKillOnExit(TRUE);

		if (!CreateProcess(pDBGKrnl->m_strExePath, pDBGKrnl->m_strParam.GetBuffer(), 
			NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, pDBGKrnl->m_strRunDir, &si, &pi))
		{
			pDBGKrnl->m_pMainFrm->MessageBox(_T("创建调试进程失败！！"),NULL,MB_OK | MB_ICONERROR);
			return 1;
		}
// 		pDBGKrnl->m_hDbgProcess = pi.hProcess;
 		CloseHandle(pi.hThread);
 		CloseHandle(pi.hProcess);
	}
	DEBUG_EVENT DBGEvent;
	pDBGKrnl->m_bExit = FALSE;
	while (!pDBGKrnl->m_bExit)
	{
		WaitForDebugEvent(&DBGEvent,INFINITE);

		BOOL	bContinue = TRUE;
		switch(DBGEvent.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:	//创建被调试进程
			bContinue = pDBGKrnl->CreateProcessEvent(&DBGEvent.u.CreateProcessInfo);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			bContinue = pDBGKrnl->ExitProcessEvent(&DBGEvent.u.ExitProcess);
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			bContinue = pDBGKrnl->CreateThreadEvent(&DBGEvent.u.CreateThread);
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			bContinue = pDBGKrnl->ExitThreadEvent(&DBGEvent.u.ExitThread);
			break;

		case LOAD_DLL_DEBUG_EVENT:		//加载DLL
			bContinue = pDBGKrnl->LoadDllEvent(&DBGEvent.u.LoadDll);
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			bContinue = pDBGKrnl->UnLoadDllEvent(&DBGEvent.u.UnloadDll);
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			bContinue = pDBGKrnl->OutputDebugStringEvent(&DBGEvent.u.DebugString,DBGEvent.dwProcessId);
			break;

		case RIP_EVENT:
			bContinue = pDBGKrnl->RipEvent(&DBGEvent.u.RipInfo);
			break;

		case EXCEPTION_DEBUG_EVENT:
			bContinue = pDBGKrnl->ExceptionEvent(&DBGEvent.u.Exception);
			break;
		}
		BOOL bRet = ContinueDebugEvent(DBGEvent.dwProcessId, DBGEvent.dwThreadId, bContinue?DBG_CONTINUE:DBG_EXCEPTION_HANDLED);
		ASSERT(bRet);

	}

	pDBGKrnl->m_pMainFrm->SendMessage(WM_USER_DEBUGSTOP);

	return 0;
}

BOOL CDBGKernel::CreateProcessEvent( CREATE_PROCESS_DEBUG_INFO* CreateProcessInfo )
{
	TCHAR szOutput[MAX_PATH+20];
	TCHAR szPath[MAX_PATH];
	GetFileNameFromHandle(CreateProcessInfo->hFile,szPath);
	_stprintf(szOutput,"加载程序：“%s”\n",szPath);
	m_pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,_tcslen(szOutput),(LPARAM)szOutput);

	CloseHandle(CreateProcessInfo->hFile);
	CloseHandle(CreateProcessInfo->hProcess);
	CloseHandle(CreateProcessInfo->hThread);
	return TRUE;
}

BOOL CDBGKernel::CreateThreadEvent( CREATE_THREAD_DEBUG_INFO* CreateThread )
{
	CloseHandle(CreateThread->hThread);
	return TRUE;
}

BOOL CDBGKernel::ExitProcessEvent( EXIT_PROCESS_DEBUG_INFO* ExitProcess )
{
	TCHAR	szOut[100];
	_stprintf(szOut,"被调试进程退出,退出代码为:%d\n",ExitProcess->dwExitCode);
	m_pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,_tcslen(szOut),(LPARAM)szOut);
	m_bExit = true;
	return FALSE;
}

BOOL CDBGKernel::ExitThreadEvent( EXIT_THREAD_DEBUG_INFO* ExitThread )
{
	TCHAR	szOut[100];
	_stprintf(szOut,"线程退出,退出代码为:%d\n",ExitThread->dwExitCode);
	m_pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,_tcslen(szOut),(LPARAM)szOut);
	return TRUE;
}

BOOL CDBGKernel::LoadDllEvent( LOAD_DLL_DEBUG_INFO* LoadDll )
{
	TCHAR	szPath[MAX_PATH];
	GetFileNameFromHandle(LoadDll->hFile,szPath);

	TCHAR	szOutput[MAX_PATH+50];
	_stprintf(szOutput,_T("加载模块:\"%s\"\n"),szPath);

	//Scintilla只支持UTF-8和ANSI，太蛋疼了
	m_pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,(WPARAM)_tcslen(szOutput),(LPARAM)szOutput);
	CloseHandle(LoadDll->hFile);
	return TRUE;
}

BOOL CDBGKernel::UnLoadDllEvent( UNLOAD_DLL_DEBUG_INFO* UnloadDll )
{

	return TRUE;
}

BOOL CDBGKernel::OutputDebugStringEvent( OUTPUT_DEBUG_STRING_INFO* DebugString, DWORD dwPID )
{
	int nStrByte = DebugString->nDebugStringLength;		//字符串的长度（字节）
	BYTE* pBuffer = new BYTE[nStrByte];
	char* szOutString = new char[nStrByte+50];

	SIZE_T bytesRead;

// 	HANDLE hDbgProcess = OpenProcess(PROCESS_VM_READ,FALSE,dwPID);
// 	ReadProcessMemory(hDbgProcess,DebugString->lpDebugStringData,
// 		pBuffer, DebugString->nDebugStringLength,&bytesRead);
// 	CloseHandle(hDbgProcess);

	if (!Toolhelp32ReadProcessMemory(dwPID,DebugString->lpDebugStringData,pBuffer,
		DebugString->nDebugStringLength,&bytesRead))
	{
		strcpy(szOutString,_T("获取调试字符串失败"));
		m_pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,strlen(szOutString),(LPARAM)szOutString);
		return TRUE;
	}

	//据说这里永远是多字节字符，
	//这个if里面的东西永远也执行不到
	//但是为了防止操蛋的微软改了，还是这么来吧
	if (DebugString->fUnicode)
	{
		char* szAnsiDbgStr = new char[nStrByte];
		WideCharToMultiByte(CP_ACP,NULL,(WCHAR*)pBuffer,-1,szAnsiDbgStr,nStrByte,NULL,FALSE);

		sprintf(szOutString,"调试字符串：“%s”\n",szAnsiDbgStr);
		delete[] szAnsiDbgStr;
	}
	else
	{
		sprintf(szOutString,"调试字符串：“%s”\n",pBuffer);
	}

	m_pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,strlen(szOutString),(LPARAM)szOutString);

	delete[] szOutString;
	delete[] pBuffer;
	return TRUE;
}

BOOL CDBGKernel::RipEvent( RIP_INFO* RipInfo )
{

	return TRUE;
}

BOOL CDBGKernel::ExceptionEvent( EXCEPTION_DEBUG_INFO* Exception )
{

	return TRUE;
}


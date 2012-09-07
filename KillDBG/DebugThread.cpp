
#include "stdafx.h"
#include "DebugThread.h"


DWORD WINAPI DebugThreadProc(LPVOID lpParameter)
{
	if (!lpParameter)
	{
		return 1;
	}

	CMainFrame*	pMainFrm = (CMainFrame*)lpParameter;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {0};
	GetStartupInfo(&si);
	DebugSetProcessKillOnExit(TRUE);

	if (!CreateProcess(pMainFrm->m_strExePath, pMainFrm->m_strParam.GetBuffer(), 
		NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, pMainFrm->m_strRunDir, &si, &pi))
	{
		pMainFrm->MessageBox(_T("创建调试进程失败！！"),NULL,MB_OK | MB_ICONHAND);
		return 0;
	}


	DEBUG_EVENT DBGEvent;
	while (!pMainFrm->m_bExit)
	{
		WaitForDebugEvent(&DBGEvent,INFINITE);

		switch(DBGEvent.dwDebugEventCode)	//we just cleanup here, but this method may have performance issue
		{
		case CREATE_PROCESS_DEBUG_EVENT:	//创建被调试进程
			TCHAR szOutput[MAX_PATH+20];
			TCHAR szPath[MAX_PATH];
			GetFileNameFromHandle(DBGEvent.u.CreateProcessInfo.hFile,szPath);
			_stprintf(szOutput,"加载程序：“%s”\n",szPath);
			pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,_tcslen(szOutput),(LPARAM)szOutput);

			CloseHandle(DBGEvent.u.CreateProcessInfo.hFile);
			CloseHandle(DBGEvent.u.CreateProcessInfo.hProcess);
			CloseHandle(DBGEvent.u.CreateProcessInfo.hThread);
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			CloseHandle(DBGEvent.u.CreateThread.hThread);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,17,(LPARAM)"收到进程退出消息\n");
			pMainFrm->m_bExit = true;
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			break;

		case LOAD_DLL_DEBUG_EVENT:		//加载DLL
			{
				TCHAR	szPath[MAX_PATH];
				GetFileNameFromHandle(DBGEvent.u.LoadDll.hFile,szPath);

				TCHAR	szOutput[MAX_PATH+50];
				_stprintf(szOutput,_T("加载模块:\"%s\"\n"),szPath);

				//Scintilla只支持UTF-8和ANSI，太蛋疼了
				pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,(WPARAM)_tcslen(szOutput),(LPARAM)szOutput);
				CloseHandle(DBGEvent.u.LoadDll.hFile);
			}
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			::MessageBox(NULL,NULL,NULL,MB_OK);
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			{
				int nStrByte = DBGEvent.u.DebugString.nDebugStringLength;		//字符串的长度（字节）
				BYTE* pBuffer = new BYTE[nStrByte];
				char* szOutString = new char[nStrByte+50];

				SIZE_T bytesRead;

				ReadProcessMemory(pi.hProcess,DBGEvent.u.DebugString.lpDebugStringData,
					pBuffer, DBGEvent.u.DebugString.nDebugStringLength,&bytesRead);

				//据说这里永远是多字节字符，
				//这个if里面的东西永远也执行不到
				//但是为了防止操蛋的微软改了，还是这么来吧
				if (DBGEvent.u.DebugString.fUnicode)
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

				pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,strlen(szOutString),(LPARAM)szOutString);

				delete[] szOutString;
				delete[] pBuffer;
			}
			break;

		case RIP_EVENT:
			break;

		case EXCEPTION_DEBUG_EVENT:
			break;
		}
		ContinueDebugEvent(DBGEvent.dwProcessId, DBGEvent.dwThreadId, DBG_CONTINUE);
	}

	pMainFrm->m_wndOutput.SendMessage(SCI_APPENDTEXT,15,(LPARAM)"被调试进程退出\n");
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;
}

#define BUFSIZE 512

BOOL GetFileNameFromHandle(HANDLE hFile, TCHAR* pszFilename) 
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
				TCHAR szTemp[BUFSIZE];
				szTemp[0] = '/0';

				if (GetLogicalDriveStrings(BUFSIZE-1, szTemp)) 
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
						if (QueryDosDevice(szDrive, szName, BUFSIZE))
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
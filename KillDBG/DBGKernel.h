#pragma once

class CMainFrame;

class CDBGKernel
{
public:
	CDBGKernel(void);
	~CDBGKernel(void);
	static DWORD WINAPI DebugThreadProc(LPVOID lpParameter);
	BOOL CreateDbgProcess(CString strExePath, CString strParam, CString strRunDir);
	BOOL AttachDbgProcess(DWORD dwPID);
	static BOOL GetFileNameFromHandle(HANDLE hFile, TCHAR* pszFilename);

	CMainFrame* m_pMainFrm;
	BOOL	m_bAttach;
	DWORD	m_dwPID;
	CString m_strExePath;
	CString m_strParam;
	CString m_strRunDir;

	HANDLE	m_hDebugThread;

	BOOL	m_bExit;

	HANDLE	m_hDbgProcess;

	BOOL CreateProcessEvent(CREATE_PROCESS_DEBUG_INFO* CreateProcessInfo);
	BOOL CreateThreadEvent(CREATE_THREAD_DEBUG_INFO* CreateThread);
	BOOL ExitProcessEvent(EXIT_PROCESS_DEBUG_INFO* ExitProcess);
	BOOL ExitThreadEvent(EXIT_THREAD_DEBUG_INFO* ExitThread);
	BOOL LoadDllEvent(LOAD_DLL_DEBUG_INFO* LoadDll);
	BOOL UnLoadDllEvent(UNLOAD_DLL_DEBUG_INFO* UnloadDll);
	BOOL OutputDebugStringEvent(OUTPUT_DEBUG_STRING_INFO* DebugString);
	BOOL RipEvent(RIP_INFO* RipInfo);
	BOOL ExceptionEvent(EXCEPTION_DEBUG_INFO* Exception);

};


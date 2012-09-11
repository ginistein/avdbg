// msvcrt.h

extern "C" {
	
	struct _startupinfo
	{
	   int newmode;
	} ;
	
	void	__cdecl __set_app_type(int Type);
	void 	__cdecl exit(int);
	void	__cdecl __getmainargs(int *, char ***, char ***, int, _startupinfo *);
	
	int		__cdecl _controlfp();
	int		__cdecl _except_handler3();
	int		__cdecl _adjust_fdiv();
	int		__cdecl __setusermatherr();
	int		__cdecl _initterm();
	int		__cdecl _acmdln();
	int		__cdecl _cexit();
	int		__cdecl _exit();
	int		__cdecl _c_exit();
	int		__cdecl srand();
	int		__cdecl rand();
	int		__cdecl __mb_cur_max();
	int		__cdecl __p__environ();
	int		__cdecl _assert();
	int		__cdecl _ctype();
	int		__cdecl _errno();
	int		__cdecl _filelengthi64();
	int		__cdecl _fstati64();
	
	int		__cdecl _iob();
	int		__cdecl _isctype();
	int		__cdecl _lseeki64();
	int		__cdecl _pctype();
	int		__cdecl _vsnprintf();
	int		__cdecl rand();
	
	FILE* __stdcall _fdopen(int fd, const char *mode ); 
	FILE* __stdcall _wfdopen(int fd, const wchar_t *mode ); 
	
	int __stdcall ungetc(char c, FILE *stream);
	int	__stdcall abort();
	int	__stdcall atexit(DWORD func);
	int __stdcall fclose(FILE *stream); 
	int __stdcall fflush(FILE *stream);
	int __stdcall fgetpos(FILE *stream,DWORD filepos); 
	FILE * __stdcall fopen(const char * path,const char * mode);
	
	
	int  __cdecl fprintf(FILE *stream,char *format,...);
	int  __stdcall fread( void *buffer, size_t size, size_t num, FILE *stream );
	int  __stdcall fsetpos(FILE* stream, const fpos_t* ptr);
	void __stdcall free(void* p);
	size_t fwrite(const void* ptr, size_t size, size_t nobj, FILE* stream);
	int getc(FILE* stream);
	struct lconv* localeconv();
	void* malloc(size_t size);
	void* memchr(const void* cs, int c, size_t n);
	void* memmove(void* s, const void* ct, size_t n);
	int putc(int c, FILE* stream);
	char* setlocale(int category, const char* locale);
	int setvbuf(FILE* stream, char* buf, int mode, size_t size);
	//void (*signal(int sig, void (*handler)(int)))(int); 
	size_t strftime(char* s, size_t smax, const char* fmt, const struct tm* tp); 
	double strtod(const char* s, char** endp); 
	int system(const char* s);

	int __stdcall _AfxChildWindowFromPoint(DWORD hWnd,DWORD point);
	int __stdcall _AfxDispatchCmdMsg(DWORD pTarget,DWORD nID,DWORD nCode3,DWORD pfn,DWORD pExtra,DWORD nSig,DWORD pHandleInfo);
	int __stdcall _AfxHandleActivate(DWORD pWnd,DWORD nState,DWORD pWndOther);
	int __stdcall _AfxHandleSetCursor(DWORD pWnd,DWORD lParam,DWORD lParam);
	int __stdcall _AfxModifyStyle(DWORD hWnd,DWORD dwRemove,DWORD dwAdd,DWORD nFlags);
	int __stdcall _AfxPostInitDialog(DWORD pWnd,DWORD rectOld,DWORD dwStyleOld);
	int __stdcall _AfxPreInitDialog(DWORD pWnd,DWORD rectOld,DWORD dwStyleOld);
	int __stdcall _CIpow();
	int __stdcall _CRT_RTC_INIT(DWORD res0,DWORD res1,DWORD res2,DWORD res3,DWORD res4);
	int __stdcall _Createsorteddata(DWORD lp_SortedData,DWORD strName,DWORD itemsize,DWORD nmax,DWORD lpsortfunc,DWORD lpdestfunc);
	int __stdcall _Decompress(DWORD strBufin,DWORD nbufin,DWORD strBufout,DWORD nbufout);
	int __stdcall _Defaultbar(DWORD lpt_bar);
	int __stdcall _Deleteruntrace();
	int __stdcall _Deletesorteddatarange(DWORD lpSortedData,DWORD AddrOne,DWORD AddrTwo);
	int __stdcall _Destroysorteddata(DWORD lp_SortedData);
	int __stdcall _Dumpbackup(DWORD lpt_dump,DWORD iAction);
	int __stdcall _Findmodule(DWORD Addr);
	int __stdcall _Findthread(DWORD threadid);
	int __stdcall _Readcommand(DWORD ip,DWORD strCmd);
	int __stdcall _Tablefunction(DWORD lpt_table,DWORD hwnd,DWORD msg,DWORD wParam,DWORD lParam);
	int __stdcall _XcptFilter(DWORD ExceptionCode,DWORD ExceptionInformation);
	int __stdcall ___crtGetEnvironmentStringsA();
	int __stdcall ___initmbctable();
	int __stdcall ___initstdio();
	int __stdcall __cinit();
	int __stdcall __crtCompareStringA(DWORD Locale,DWORD dwCmpFlags,DWORD lpString1,DWORD cchCount1,DWORD lpString2,DWORD cchCount2);
	int __stdcall __dllonexit(DWORD arg1,DWORD arg2,DWORD arg3);
	int __stdcall __fpmath();
	int __stdcall __getmainargs(DWORD argCount,DWORD lpCommand,DWORD lpUser);
	int __stdcall __initptd(DWORD ptd);
	int __stdcall __ioinit();
	int __stdcall __mtinit();
	int __stdcall __p__commode();
	int __stdcall __p__fmode();
	int __stdcall __set_app_type(DWORD Type);
	int __stdcall __setargv(DWORD argc,DWORD argv);
	int __stdcall __setenvp();
	int __stdcall __setmbcp(DWORD codepage);
	int __stdcall __threadid();
	int __stdcall __wgetmainargs(DWORD argc,DWORD argv,DWORD envp,DWORD dowildcard,DWORD startinfo);
	int __stdcall _beginthreadex(DWORD security,DWORD stack_size,DWORD dwStart_Address,DWORD arglist,DWORD initflag,DWORD thrdaddr);
	int __stdcall _filelength(DWORD handle);
	int __stdcall _fileno(DWORD stream);
	int __stdcall _getch();
	int __stdcall _getpid();
	int __stdcall _initterm(DWORD pfbegin,DWORD pfend);
	int __stdcall _isatty(DWORD handle);
	int __stdcall _itoa(DWORD value,DWORD lpBuffer,DWORD dwSize);
	int __stdcall _lock(DWORD arg1);
	int __stdcall _mbscat(DWORD dest,DWORD src);
	int __stdcall _mbscmp(DWORD string1,DWORD string2);
	int __stdcall _mbscpy(DWORD lpBuffer,DWORD string);
	int __stdcall _mbsdup(DWORD string);
	int __stdcall _mbsicoll(DWORD szString1,DWORD szString2);
	int __stdcall _mbsinc(DWORD string);
	int __stdcall _mbsrchr(DWORD szStr,DWORD char);
	int __stdcall _msize(DWORD ptr);
	int __stdcall _onexit(DWORD Func);
	int __stdcall _open(DWORD lpPathName,DWORD iReadWrite);
	int __stdcall _parse_cmdline(DWORD argc,DWORD argv);
	int __stdcall _read(DWORD handle,DWORD lpBuffer,DWORD nBufferSize);
	int __stdcall _setmbcp(DWORD CodePage);
	int __stdcall _setmode(DWORD handle,DWORD mode);
	int __stdcall _snprintf(DWORD lpBuffer,DWORD nSize,DWORD szformat,DWORD Arg1);
	int __stdcall _strcmpi(DWORD lpString1,DWORD lpString2);
	int __stdcall _unlink(DWORD szPathName);
	int __stdcall _unlock(DWORD arg1);
	int __stdcall _write(DWORD handle,DWORD szBuffer,DWORD nbyte);


}
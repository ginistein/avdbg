typedef char *LPSTR, *PSTR;
typedef char *LPCSTR;
typedef char TCHAR;

typedef LPSTR PTSTR;
typedef LPSTR LPTSTR;

typedef const char* LPCTSTR;

typedef void *LPWSTR, *PWSTR;
typedef const void *LPCWSTR, *PCWSTR;

typedef int   BOOL;
typedef int   bool;
typedef int   INT;
typedef int   wchar_t;

typedef float FLOAT;
typedef float* PFLOAT,LPFLOAT;

typedef long LONG;
typedef long LRESULT;

typedef BYTE * LPBYTE;
typedef BYTE * PBYTE;
typedef BYTE BCHAR;

typedef void *PVOID;
typedef void *LPVOID;
typedef const void far   *LPCVOID;
typedef void VOID;

typedef PVOID HANDLE;

typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef HANDLE *PHANDLE;
typedef HANDLE HLOCAL;
typedef HANDLE HACCEL;
typedef HANDLE HICON;
typedef HANDLE HMENU;
typedef HANDLE HCURSOR;
typedef HANDLE HBRUSH;
typedef HANDLE HDC;
typedef HANDLE HMODULE;

typedef HANDLE KeyboardLayoutHandle;
typedef HANDLE HHOOK;
typedef HANDLE HDESK;
typedef HANDLE HGESTUREINFO;
typedef HANDLE HTOUCHINPUT;
typedef HANDLE HWINSTA;
typedef HANDLE HBITMAP;
typedef HANDLE HCONV;
typedef HANDLE HDDEDATA;
typedef HANDLE HSZ;
typedef HANDLE HCONVLIST;
typedef HANDLE HDWP;
typedef HANDLE HRGN;
typedef HANDLE HKL;
typedef HANDLE HMONITOR;
typedef HANDLE HRAWINPUT;
typedef HANDLE * LPHANDLE;

typedef HANDLE HMIDIOUT;
typedef HANDLE HMIDI;
typedef HANDLE HMIDIIN;
typedef HANDLE HMIDISTRM;
typedef HANDLE HMIXER;
typedef HANDLE HMIXEROBJ;

typedef DWORD size_t;
typedef DWORD UINT;
typedef DWORD COLORREF;

typedef DWORD  ATOM;

typedef UINT WPARAM;
typedef long LPARAM;
typedef DWORD far  *LPDWORD;
typedef WORD* LPWORD;

typedef long* PLONG,LPLONG;
typedef unsigned long ULONG;
typedef unsigned long* PULONG;
typedef unsigned short USHORT;
typedef unsigned short* PUSHORT;
typedef unsigned char UCHAR;
typedef unsigned char* PUCHAR;

typedef int* LPINT;
typedef UINT UINT_PTR;
typedef LONG LONG_PTR;
typedef INT  INT_PTR;
typedef ULONG ULONG_PTR;
typedef ULONG* PULONG_PTR;
typedef DWORD DWORD_PTR;
typedef DWORD* PDWORD;
typedef DWORD* PDWORD_PTR;
typedef UINT* PUINT_PTR;
typedef BOOL* PBOOL,*LPBOOL;

typedef DWORD   FILE;
typedef FILE *  PFILE;
typedef DWORD   fpos_t;

typedef struct tagRECT
{
    int    left;
    int    top;
    int    right;
    int    bottom;
}RECT,* LPRECT;

typedef struct tagPOINT
{
    long  x;
    long  y;
}POINT,* LPPOINT;

typedef struct tagSIZE {
    LONG cx; 
    LONG cy; 
}SIZE,*LPSIZE;

typedef struct tagACCEL { // accl  
    BYTE   fVirt; 
    WORD   key; 
    WORD   cmd; 
} ACCEL,*LPACCEL;

typedef const RECT* LPCRECT;


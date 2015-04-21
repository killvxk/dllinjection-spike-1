#include "dllinjector.h"

// Structure used store state in EnumWindowsProc
typedef struct _PROCESSWINDOWINFO {
    DWORD dwProcessId;
    DWORD dwThreadId;
    HWND hWnd;
} PROCESSWINDOWINFO, *LPPROCESSWINDOWINFO;

// Application-defined callback for EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {

  // Get state
  LPPROCESSWINDOWINFO lppwi = (LPPROCESSWINDOWINFO)lParam;
  
  // Get process and thread ids for window
  DWORD dwProcessId = 0;
  DWORD dwThreadId = 0;
  dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
  
  // Is the window in the desired process, and is it the top level window?
  if (
    dwThreadId != 0 &&
    dwProcessId != 0 && 
    (lppwi->dwProcessId == dwProcessId) && 
    (GetWindow(hWnd, GW_OWNER) == (HWND)0) && 
    IsWindowVisible(hWnd)
  ) {
    // Update state
    lppwi->dwThreadId = dwThreadId;  
    lppwi->hWnd = hWnd;
    // Stop enumeration
    return FALSE;
  }

  // Continue enumeration
  return TRUE;
}

// Gets a processes main window handle and thread id
HWND GetProcessMainWindowHandle(DWORD dwProcessId, LPDWORD lpdwThreadId) {
  
  PROCESSWINDOWINFO pwi = {0};
  pwi.dwProcessId = dwProcessId;
  
  // Enumerate all top level windows and find the main window for the given process
  if ( (EnumWindows(EnumWindowsProc, (LPARAM)&pwi) == FALSE) && (pwi.dwThreadId != 0) && (pwi.hWnd != NULL) ) {    
    if(lpdwThreadId != NULL) {
      *lpdwThreadId = pwi.dwThreadId;
    }
    return pwi.hWnd;
  }
  
  return NULL;
}

// Implementation of the SetWindowsHookEx DLL injection technique. This approach
// only works on processes that have a window. 
BOOL InjectDll2(DWORD dwProcessId, wchar_t* pwszModuleFileName) {
  
  BOOL bSuccess = FALSE;
  HMODULE hModule = NULL;
  HOOKPROC lpfnHookProc = NULL;
  HWND hRemoteWindow = NULL;
  DWORD dwRemoteWindowThreadId = 0;
  HHOOK hHook = NULL;

  do {

    // Load the DLL into this process's address space
    hModule = LoadLibrary(pwszModuleFileName);
    if(hModule == NULL) {
      printf("LoadLibrary error: %d \r\n", GetLastError());
      break;
    }

    // Get the address of the hook procedure
    lpfnHookProc = (HOOKPROC)GetProcAddress(hModule, "_CallWndProc@12");
    if(lpfnHookProc == NULL) {
      printf("GetProcAddress error: %d \r\n", GetLastError());
      break;
    }

    // Get the window handle and thread id for the processes' main window
    hRemoteWindow = GetProcessMainWindowHandle(dwProcessId, &dwRemoteWindowThreadId);
    if(hRemoteWindow == NULL || dwRemoteWindowThreadId == 0) {
      printf("GetProcessMainWindowHandle error: %d \r\n", GetLastError());
      break;
    }

    // Install a thread-specific hook procedure in the remote thread's hook chain
    hHook = SetWindowsHookEx(WH_CALLWNDPROC, lpfnHookProc, hModule, dwRemoteWindowThreadId);
    if(hHook == NULL) {
      printf("SetWindowsHookEx error: %d \r\n", GetLastError());
      break;
    }

    // The DLL containing our hook procedure is loaded by the remote thread's 
    // message loop. Sending a WM_NULL message forces the DLL to be loaded into
    // the address space of the remote process.
    SendMessage(hRemoteWindow, WM_NULL, 0, 0);

    // Remove the hook procedure from the remote thread's hook chain
    if(UnhookWindowsHookEx(hHook) == 0) {
      printf("UnhookWindowsHookEx error: %d \r\n", GetLastError());
      break;
    }

    // Force remote thread's message loop to unload the DLL containing the hook
    // Sprocedure
    SendMessage(hRemoteWindow, WM_NULL, 0, 0);

    // Unload the DLL from this processes' address space
    if(FreeLibrary(hModule) == 0) {
      printf("UnhookWindowsHookEx error: %d \r\n", GetLastError());
      break;
    }
    hModule = NULL;

    bSuccess = TRUE;

  } while(0);

  return bSuccess;
}
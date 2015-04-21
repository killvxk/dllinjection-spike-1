#include "dllinjector.h"

BOOL TryAdjustTokenPrivileges(wchar_t* lpName, DWORD dwAtributes) {
    
  BOOL bSuccess = FALSE;
  HANDLE hToken = NULL;
  LUID luid = {0};
  TOKEN_PRIVILEGES tp = {0};

  do {

    if(OpenProcessToken(GetCurrentProcess(),(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY),&hToken) == 0) {
      printf("OpenProcessToken error: %d \r\n", GetLastError());
      break;
    }
    
    if(LookupPrivilegeValue(NULL,lpName,&luid) == 0) {
      printf("LookupPrivilegeValue error: %d \r\n", GetLastError());
      break;
    }
        
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = dwAtributes; 
    if(AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL) == 0) {
      printf("AdjustTokenPrivileges error: %d \r\n", GetLastError());
      break;
    }

    bSuccess = TRUE;

  } while(0);

  if(hToken != NULL) {
    CloseHandle(hToken);
    hToken = NULL;
  }

  return bSuccess;
}

BOOL InjectDll1(DWORD dwProcessId, wchar_t* pwszModuleFileName);
BOOL InjectDll2(DWORD dwProcessId, wchar_t* pwszModuleFileName);
BOOL InjectDll3(DWORD dwProcessId, wchar_t* pwszModuleFileName);

#define InjectDLL InjectDll1

int wmain(int argc, wchar_t* argv[])
{
  STARTUPINFO si = {0};
  PROCESS_INFORMATION pi = {0};

  do {

    // To open a process owned by another user (including SYSTEM) you must hold
    // the SE_DEBUG priveledge. Normally, SE_DEBUG is only granted to members of
    // the Administrator group. However, even if you are running as a normal user,
    // you can still inject into another process that you own.
    TryAdjustTokenPrivileges(SE_DEBUG_NAME, SE_PRIVILEGE_ENABLED);
  
    if(CreateProcess(L"target.exe", NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == 0) {
        printf("CreateProcess error: %d \r\n", GetLastError());
        return 0;
    }

    // Wait until the first GUI thread in the process becomes idle. This is not 
    // necessarily the thread with the main window - thus the sleep as a precaution.
    WaitForInputIdle(pi.hProcess, INFINITE);
    Sleep(1000);
    
    // Inject DLL
    printf("Injecting DLL into process \r\n");
    InjectDLL(pi.dwProcessId, L"dll.dll");

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);
    printf("Process has exited \r\n");

  } while(0);

  // Cleanup
  if(pi.hProcess != NULL) {
    CloseHandle(pi.hProcess);
    pi.hProcess = NULL;
  }
  if(pi.hThread != NULL) {
    CloseHandle(pi.hThread);
    pi.hThread = NULL;
  }

	return 0;
}


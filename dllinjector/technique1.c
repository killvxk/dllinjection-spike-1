#include "dllinjector.h"

// Implementation of the CreateRemoteThread/LoadLibrary DLL injection technique
BOOL InjectDll1(DWORD dwProcessId, wchar_t* pwszModuleFileName) {
  
  BOOL bSuccess = FALSE;
  HANDLE hProcess = NULL;
  HANDLE hRemoteThread = NULL;
  LPVOID pwszRemoteString = NULL;
  LPVOID fnLoadLibrary = NULL;
  size_t cbModuleFileName = 0;

  do {

    // Open the target process
    hProcess = OpenProcess((PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION), FALSE, dwProcessId);
    if(hProcess == NULL) {
      printf("OpenProcess error: %d \r\n", GetLastError());
      break;
    }

    // Get the size of the wchar string in bytes
    cbModuleFileName = (wcslen(pwszModuleFileName) + 1) * sizeof(wchar_t);

    // Get the address of kernel32.dll!LoadLibraryW in the current process. The 
    // load addresses of system DLLs like kernel32.dll are randomized on each 
    // boot, but are the same across all processes of the same architecture 
    // (i.e. x86 or x64) afterwards. Thus, this address will be the same in the
    // target process. 
    fnLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
    if(fnLoadLibrary == NULL) {
      printf("GetProcAddress error: %d \r\n", GetLastError());
      break;
    }
  
    // Reserves and commits a read/write region of memory within the virtual address space 
    // of the target process to store the module file name.
    pwszRemoteString = VirtualAllocEx(hProcess, NULL, cbModuleFileName, (MEM_RESERVE | MEM_COMMIT), PAGE_READWRITE);
    if(pwszRemoteString == NULL) {
      printf("VirtualAllocEx error: %d \r\n", GetLastError());
      break;
    }
  
    // Copy the module file name to the target process
    if(WriteProcessMemory(hProcess, pwszRemoteString, pwszModuleFileName, cbModuleFileName, NULL) == 0) {
      printf("WriteProcessMemory error: %d \r\n", GetLastError());
      break;
    }

    // Create a thread that runs in the virtual address space of another process.
    hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)fnLoadLibrary, pwszRemoteString, 0, NULL);
    if(hRemoteThread == NULL) {
      printf("CreateRemoteThread error: %d \r\n", GetLastError());
      break;
    }
      
    // NOTE: DLL will remain loaded in address space of remote process until it 
    // unloads itself.
    bSuccess = TRUE;
  
  } while(0);

  // Cleanup
  if(hProcess != NULL) {
    CloseHandle(hProcess);
    hProcess = NULL;
  }
  if(hRemoteThread != NULL) {
    CloseHandle(hRemoteThread);
    hProcess = NULL;
  }

  return bSuccess;
}
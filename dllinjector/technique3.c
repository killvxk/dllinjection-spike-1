#include "dllinjector.h"
#include <TlHelp32.h>

#define PAYLOAD_OFFSET_RETURN_ADDRESS 1
#define PAYLOAD_OFFSET_LOADLIBRARY_ARG_ADDRESS 8
#define PAYLOAD_OFFSET_LOADLIBRARY_ADDRESS 13

__declspec(naked) void Payload(void) {
  _asm {
    
    // Placeholder for the return address
    push 0xDEADBEEF

    // Save the flags and registers
    pushfd
    pushad

    // Placeholder for the string address and LoadLibrary
    push 0xDEADBEEF
    mov eax, 0xDEADBEEF

    // Call LoadLibrary with the string parameter
    call eax

    // Restore the registers and flags
    popad
    popfd

    // Return control to the hijacked thread
    ret
  }
}

__declspec(naked) void Payload_END(void)
{
}

DWORD GetFirstRemoteThreadId(DWORD dwProcessId) {

  DWORD dwThreadId = 0;
  HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
  THREADENTRY32 te32 = {0}; 

  do {

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // Take a snapshot of all running threads in the system 
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if(hThreadSnap == INVALID_HANDLE_VALUE) {
      printf("CreateToolhelp32Snapshot error: %d \r\n", GetLastError());
      break;
    }
  
    // Retrieve information about the first thread in the system
    if(Thread32First(hThreadSnap, &te32) == FALSE) {
      printf("Thread32First error: %d \r\n", GetLastError());
      break;
    }

    // Find the first thread running in the desired process
    do 
    { 
      if(te32.th32OwnerProcessID == dwProcessId)
      {
        dwThreadId = te32.th32ThreadID;
        break;
      }
    } while(Thread32Next(hThreadSnap, &te32 ) == TRUE);

  } while(0);

  // Cleanup
  if(hThreadSnap != INVALID_HANDLE_VALUE) {
    CloseHandle(hThreadSnap);
    hThreadSnap = INVALID_HANDLE_VALUE;
  }

  return dwThreadId;
}

// Implementation of the thread hijacking DLL injection technique.
BOOL InjectDll3(DWORD dwProcessId, wchar_t* pwszModuleFileName) {
  
  BOOL bSuccess = FALSE;
  HANDLE hProcess = NULL;
  HANDLE hRemoteThread = NULL;
  LPVOID fnRemotePayload = NULL;
  LPVOID fnLoadLibrary = NULL;
  LPVOID pwszRemoteString = NULL;
  DWORD dwRemoteThreadId = 0;
  size_t cbModuleFileName = 0;
  CONTEXT ctx = {0};
  size_t cbPayload = 0;

  do {
   
    // Open the target process
    hProcess = OpenProcess((PROCESS_VM_OPERATION | PROCESS_VM_WRITE), FALSE, dwProcessId);
    if(!hProcess) {
      printf("OpenProcess error: %d \r\n", GetLastError());
      break;
    }

    // Get the size of the wchar string in bytes
    cbModuleFileName = (wcslen(pwszModuleFileName) + 1) * sizeof(wchar_t);

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

    // Calculate the size in bytes of the payload
    cbPayload = (DWORD)Payload_END - (DWORD)Payload;

    // Reserves and commits a read/write region of memory within the virtual address space 
    // of the target process to store the Payload.
    fnRemotePayload = VirtualAllocEx(hProcess, NULL, cbPayload, (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
    if(fnRemotePayload == NULL) {
      printf("VirtualAllocEx error: %d \r\n", GetLastError());
      break;
    }
    
    // Copy the Payload to the target process
    if(WriteProcessMemory(hProcess, fnRemotePayload, Payload, cbPayload, NULL) == 0) {
      printf("WriteProcessMemory error: %d \r\n", GetLastError());
      break;
    }

    // Copy pointer to the DLL name to the target process
    if(WriteProcessMemory(hProcess, (void*)((char*)fnRemotePayload + PAYLOAD_OFFSET_LOADLIBRARY_ARG_ADDRESS), &pwszRemoteString, sizeof(DWORD), NULL) == 0) {
      printf("WriteProcessMemory error: %d \r\n", GetLastError());
      break;
    }

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

    // Copy the address of load library to the target process
    if(WriteProcessMemory(hProcess, (void*)((char*)fnRemotePayload + PAYLOAD_OFFSET_LOADLIBRARY_ADDRESS), &fnLoadLibrary, sizeof(DWORD), NULL) == 0) {
      printf("WriteProcessMemory error: %d \r\n", GetLastError());
      break;
    }

    // Get the thread id of a random thread in the process
    dwRemoteThreadId = GetFirstRemoteThreadId(dwProcessId);
    if(dwRemoteThreadId == 0) {
      printf("GetFirstRemoteThreadId error: %d \r\n", GetLastError());
      break;
    }

    // Open handle to remote thread
    hRemoteThread = OpenThread((THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME), FALSE, dwRemoteThreadId);
    if(hRemoteThread == NULL) {
      printf("OpenThread error: %d \r\n", GetLastError());
      break;
    }

    // Suspend remote thread
    if(SuspendThread(hRemoteThread) == -1) {
      printf("SuspendThread error: %d \r\n", GetLastError());
      break;
    }

    // Get thread context
    ctx.ContextFlags = CONTEXT_CONTROL;
    if(GetThreadContext(hRemoteThread, &ctx) == 0) {
      printf("GetThreadContext error: %d \r\n", GetLastError());
      break;
    }

    // Copy the current value of the EIP register to the target process
    if(WriteProcessMemory(hProcess, (void*)((char*)fnRemotePayload + PAYLOAD_OFFSET_RETURN_ADDRESS), &ctx.Eip, sizeof(DWORD), NULL) == 0) {
      printf("WriteProcessMemory error: %d \r\n", GetLastError());
      break;
    }

    // Update the thread context
    ctx.Eip = (DWORD)fnRemotePayload;
    ctx.ContextFlags = CONTEXT_CONTROL;
    if(SetThreadContext(hRemoteThread, &ctx) == 0) {
      printf("SetThreadContext error: %d \r\n", GetLastError());
      break;
    }

    // Flush the instruction cache
    if(FlushInstructionCache(hProcess, fnRemotePayload, cbPayload) == 0) {
     printf("FlushInstructionCache error: %d \r\n", GetLastError());
      break;
    }
    
    // Resume execution of remote thread
    if(ResumeThread(hRemoteThread) == -1) {
      printf("ResumeThread error: %d \r\n", GetLastError());
      break;
    }

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
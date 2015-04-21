#include "dll.h"

__declspec(dllexport) LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {

  printf("In dll.dll!CallWndProc() \r\n");

  // Pass event to the next hook procedure in the hook chain
  return CallNextHookEx(NULL, nCode, wParam, lParam);

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	
  printf("In dll.dll!DllMain(): ");
  switch (ul_reason_for_call) {
	  case DLL_PROCESS_ATTACH:
      printf("DLL_PROCESS_ATTACH \r\n");
      break;
	  case DLL_THREAD_ATTACH:
      printf("DLL_THREAD_ATTACH \r\n");
      break;
	  case DLL_THREAD_DETACH:
      printf("DLL_THREAD_DETACH \r\n");
      break;
	  case DLL_PROCESS_DETACH:
      printf("DLL_PROCESS_DETACH \r\n");
      break;
	}

	return TRUE;
}



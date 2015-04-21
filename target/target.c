#include "target.h"

#define WINDOW_CLASS_NAME L"Dummy Hidden Window"

// Receives and processes all messages sent to the hidden window.
// Every window class has a window procedure, and every window created with 
// that class uses that same window procedure to respond to messages.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  LRESULT lresult = 0;
  switch (uMsg) {
    case WM_DESTROY:
      // An application can end its own loop by using the PostQuitMessage functio
      PostQuitMessage(0); // WPARAM
      break;
    default:
      // Send the message back to the system for default processing
      lresult = DefWindowProc(hWnd, uMsg, wParam, lParam);
      break;
  }
  return lresult;
}

// Creates a hidden window and starts pumping messages on the calling thread.
// The system maintains a single system message queue and one thread-specific 
// message queue for each GUI thread. To avoid the overhead of creating a 
// message queue for non–GUI threads, all threads are created initially without
// a message queue. The system creates a thread-specific message queue only 
// when the thread makes its first call to one of the specific user functions. 
// A thread's message queue receives all mouse and keyboard messages for the 
// windows created by the thread. 
WPARAM CreateHiddenWindow()
{
  
  HWND hWnd = NULL;
  MSG msg = {0};
  WNDCLASS wndclass = {0};
  HINSTANCE hInstance = NULL; 
  
  // Get a handle to the module for this EXE 
  hInstance = GetModuleHandle(NULL);
  
  // Create and register a new window class.
  wndclass.lpfnWndProc = WindowProc;
  wndclass.hInstance = hInstance;
  wndclass.lpszClassName = WINDOW_CLASS_NAME;
  if (RegisterClass(&wndclass) == 0) {
    printf("RegisterClass error: %d \r\n", GetLastError());
    return 0;
  }
  
  // Create a window based on the class
  hWnd = CreateWindowEx(
    WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, // Hidden
    WINDOW_CLASS_NAME,
    WINDOW_CLASS_NAME,
    WS_POPUPWINDOW, 
    0, 0, 0, 0,
    NULL, 
    NULL, 
    hInstance, 
    NULL
  );
  if(hWnd == NULL) {
    printf("CreateWindowEx error: %d \r\n", GetLastError());
    return 0;
  }

  // Show the window
  ShowWindow(hWnd, SW_SHOW);

  // Run the message loop on this thread. 
  // GetMessage removes a message from this thread's message queue. The message
  // loop ends when the GetMessage function removes a WM_QUIT message. 
  while (GetMessage(&msg, NULL, 0, 0) != 0)
  {
    printf("In target.exe!CreateHiddenWindow() message loop \r\n");
    // Translates a virtual-key message into character messages
    TranslateMessage (&msg);
    // Direct the system to send the message to the window procedure associated 
    // with the window handle specified in the MSG structure.
    DispatchMessage (&msg);
  }

  return msg.wParam;
}

DWORD WINAPI ThreadProc(LPVOID lpParameter) {
  CreateHiddenWindow();
	return 0;
}

int wmain(int argc, wchar_t* argv[]) {

  // Create a hidden window on another thread
  CreateThread(NULL, 0, &ThreadProc, NULL, 0, NULL);

  // Simulate some work
  while(TRUE) {
    printf(". \r\n");
    Sleep(1000);
  }

  return 0;

}


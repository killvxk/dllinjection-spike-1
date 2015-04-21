dllinjection-spike: A spike that illustrates various DLL injection techniques

## Compile from source

Visual Studio 2012 Update 4 is required. To compile the source code run the following command:

```
build.bat
```

NOTE: Debug build currently has issues with thread hijacking DLL injection technique.

## References

* [Black Hat USA 2007 Butler and Kendall Presentation](https://www.blackhat.com/presentations/bh-usa-07/Butler_and_Kendall/Presentation/bh-usa-07-butler_and_kendall.pdf)
* [Basic Windows Shellcode Development Part 1](http://www.kdsbest.com/?p=171)
* [Dll Injection with CreateRemoteThread](http://stackoverflow.com/questions/22750112/dll-injection-with-createremotethread)
* [Writing Shellcode with a C Compiler](https://nickharbour.wordpress.com/2010/07/01/writing-shellcode-with-a-c-compiler/)
* [The Shellcoder's Handbook 2nd Edition](https://murdercube.com/files/Computers/Computer%20Security/Wiley.The.Shellcoders.Handbook.2nd.Edition.Aug.2007.pdf)
* [In pursuit of the message queue](http://blogs.msdn.com/b/oldnewthing/archive/2006/02/21/536055.aspx)
* [Creating a window](https://msdn.microsoft.com/en-us/library/windows/desktop/ff381397.aspx)
* [C++: Best way to get Window Handle of the only window from a process by process id, process handle and title name](http://stackoverflow.com/questions/20162359/c-best-way-to-get-window-handle-of-the-only-window-from-a-process-by-process)
* [May 1996 Microsoft Systems Journal - Under the Hood](https://www.microsoft.com/msj/archive/S2CE.aspx)
* [Internals of a Windows Thread](http://www.codeproject.com/Articles/662735/Internals-of-Windows-Thread)

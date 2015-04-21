call "%VS110COMNTOOLS%\VsDevCmd.bat"
msbuild dllinjection-spike.sln /t:Rebuild /p:Configuration=Release /p:Platform="Win32"

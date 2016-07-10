call findvs.bat
msbuild "Complete Hider.sln" /p:Configuration=Release /p:Platform=Win32 /t:Rebuild

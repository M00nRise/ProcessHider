---- Created by 0xM00nRise ------

-------- IMPORTANT NOTE ---------
This tools is allowed to use only for penetration testing and other white-hat activities. Malicious use is absoloutly frobidden!

------------ About  -------------
ProcessHider is a post-exploitation tool designed to hide processes from monitoring tools such as Task Manager and Process Explorer,
thus preventing the admins from discovering payload's processes. 
The tool works on both 32 and 64 bit versions, by  self detecting the OS version and using the right version of the tool.


----------- License -------------

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:


The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
(Taken from EasyHook's license)

------- External Libraries ---------
This project uses the EasyHook project, available at https://easyhook.github.io/ and Xgetopt, available at http://www.codeproject.com/Articles/1940/XGetopt-A-Unix-compatible-getopt-for-MFC-and-Win32
------ Using ProcessHider -------
call the main file, ProcessHider, which is under MainFile/, from cmd, using the following options:
-i - specify process IDs that you want to hide, seperated by commas (without space!)
-n - specify process names that you want to hide, seperated by commas (again, no spaces)
-x - specify monitoring applications you want to avoid, other than defaults (powershell.exe,taskmgr.exe,procexp.exe,procexp64.exe,perfmon.exe)

example usage:
>	ProcessHider -i 5454,3672 -n "chrome.exe,notepad.exe" -x "cmd.exe"

note: you need to use at least one of -i or -n. The hider will make sure to hide itself as well.

---- Structure and operation ----
First, the hider checks whether the OS is 32 bit or 64 bit, and chooses the right version to use.
Then, it launches a daemon, which looks for one of the frobidden monitoring tools. When it finds one - it uses DLL injection
to launch the payload - which hooks the call to NtQuerySystemInformation - the method the OS tools use to enumerate active processes,
and deletes each of the processes specified (and the daemon) from the results.
Passing processes information to the injected DLL is done by a file, whose location is hardcoded.

-------- Issues and TODOs --------
Issue #1 - 	Sometimes powershell crashes after trying to retrive process list using "Get-Process"
Issue #2 - 	Injecting right when process loads may cause it to crash (especially Process Explorer). Right now it's fixed using a short delay,
			but a better solution is needed, since the delay is sometimes noticeable.

TODO #1 -	Build a powershell version of this tool
TODO #2 - 	Create more elegant way to pass arguments to the injected DLL
TODO #3 - 	Find a way to hook calls to "tasklist.exe" - right now it's too quick (maybe also using different API call?)
TODO #4 - 	Replace the linked list usage with a c++11 template
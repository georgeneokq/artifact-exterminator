# Developer guide

Warning!!! Changes may be destructive and should be tested on a Windows VM.

Self-explanatory features such as registry backup and restoration are not documented.

## Testing features

### Registry keys/values deletion
Example command that runs notepad to open a file, and deletes the provided keys and values upon closing notepad.

WARNING: Make sure to run this in a VM!

`.\artifact-exterminator.exe -f notepad.exe --args C:\Windows\win.ini -k "HKCU\Volatile Environment\1" -v "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Reliability:LastAliveStamp,HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Reliability:TimeStampInterval"`

### Shimcache entry deletion
Download the AppCompatCacheParser tool from [this website](https://ericzimmerman.github.io/#!index.md), or any other ShimCache parser to view ShimCache entries.

1. View the ShimCache entries
2. Run the program with a specified executable, such as notepad.exe
3. Restart the machine, view the ShimCache entries again - records of the specified executable should be deleted

### Kill switch

Run the following command:
`.\artifact-exterminator.exe -f calc.exe --killswitch-ip 127.0.0.1 --killswitch-port 8080 --killswitch-poll 3`

This will run calc.exe, and start polling for the kill switch hosted on the local machine on port 8080,
with an interval of 3 seconds once the calc.exe app is closed.

On another terminal, run the [example kill switch python script](external/sock.py). The script will host a socket which serves as the kill switch, and will be activated after 6 seconds.
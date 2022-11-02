# Artifact Exterminator

This page is a user guide.

For developers, see the [developer documentation](docs/developing.md).

For a high-level overview of this project, [read this page](docs/idea.md).

## Program arguments

- *-f* File path of executable. If the executable can be found in PATH, there is no need to specify the full path. Alternatively, this argument can be any command line tool, as it is ran in the context of a shell, using `cmd /c`
- *--args* Arguments for executable specified via `-f`
 * *-d* Amount of time to delay before performing cleanup, in seconds. A sleep timer is started after the executable finishes running.
- *--killswitch-ip* IPv4 address of kill switch socket. Must be provided along with `killswitch-port` option
- *--killswitch-port* Port of remote socket. Must be provided along with `killswitch-ip` option
- *--killswitch-poll* Interval for polling, in seconds. This argument is optional; defaults to once every 10 seconds
- *-k* Registry keys to remove, comma-separated
- *-v* Registry values to remove, comma-separated. Value name should come after the key, separated by colon. e.g. `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\AppCompatCache:AppCompatCache`. The root key can be specified by either its full name or by its shorthand, like `HKLM`
- *-a* Additional file names to remove traces of, comma-separated
- *-s* Run the second part (after system reboot) of specified features, if applicable. Some artifacts such as Shimcache and Amcache are viewable only after system reboot. The value of this option is not relevant, but is still required. e.g. `artifact-exterminator.exe -s 1 -a notepad.exe,mimikatz.exe`. This argument is mainly for internal use within the program code for scheduling tasks.
- *--features* Specify features to run, comma-separated. If this argument is not provided, all features are ran by default. Possible values:
  - registry
  - shimcache
  - prefetch
  - amcache

Values should come after their flags, separated by spaces.

### Examples

#### All-in-one example: Run sample malware and remove all traces

Download [artifact-exterminator-malware](https://github.com/georgeneokq/artifact-exterminator/raw/main/external/artifact-exterminator-malware.zip), attach a .exe extension to the file name and run the following command.
```
artifact-exterminator.exe -f artifact-exterminator-malware.exe --args 15 -k "HKCU\Keyboard Layout\MaliciousKey1,HKCU\Keyboard Layout\MaliciousKey2" -v "HKCU\Control Panel\Mouse:MaliciousValue1,HKCU\Control Panel\Mouse:MaliciousValue2" --features registry,shimcache,prefetch,amcache -d 10
```

#### All-in-one example with kill switch

Run the following command with [artifact-exterminator-malware](https://github.com/georgeneokq/artifact-exterminator/raw/main/external/artifact-exterminator-malware.zip):
```
artifact-exterminator.exe -f artifact-exterminator-malware.exe --args 15 -k "HKCU\Keyboard Layout\MaliciousKey1,HKCU\Keyboard Layout\MaliciousKey2" -v "HKCU\Control Panel\Mouse:MaliciousValue1,HKCU\Control Panel\Mouse:MaliciousValue2" --features registry,shimcache,prefetch,amcache -d 10 --killswitch-ip 127.0.0.1 --killswitch-port 8080
```

When the program starts to indicate that it is attempting to connect to the specified kill switch, run the [sample kill switch](https://github.com/georgeneokq/artifact-exterminator/external/sock.py) and wait the timeout until the kill switch is activated:
```
python sock.py
```

#### Open a file with notepad and remove traces after notepad is closed
```
artifact-exterminator.exe -f notepad.exe --args C:\Windows\win.ini
```

#### Open a file with notepad and remove traces after notepad is closed, and specified kill switch is activated

```
artifact-exterminator.exe -f notepad.exe --args C:\Windows\win.ini --killswitch-ip 127.0.0.1 --killswitch-port 8080 --killswitch-poll 3
```
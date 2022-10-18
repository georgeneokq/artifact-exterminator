# Artifact Exterminator

This page is a user guide.

For developers, see the [developer documentation](docs/developing.md).

For a high-level overview of this project, [read this page](docs/idea.md).

## Program arguments

- *-f* File path of executable. If the executable can be found in PATH, there is no need to specify the full path. Alternatively, this argument can be any command line tool, as it is ran in the context of a shell, using `cmd /c`
- *--args* Arguments for executable specified via `-f`
- *--killswitch-ip* IPv4 address of kill switch socket. Must be provided along with `killswitch-port` option
- *--killswitch-port* Port of remote socket. Must be provided along with `killswitch-ip` option
- *--killswitch-poll* Interval for polling, in seconds. This argument is optional; defaults to once every 10 seconds
- *-k* Registry keys to remove, comma-separated
- *-v* Registry values to remove, comma-separated. Value name should come after the key, separated by colon. e.g. `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\AppCompatCache:AppCompatCache`. The root key can be specified by either its full name or by its shorthand, like `HKLM`
- *-a* Additional file names to remove traces of, comma-separated
- *-s* Only run shimcache removal function. The value of this option is not relevant, but is still required. e.g. `artifact-exterminator.exe -s 1 -a calc.exe,mimikatz.exe`. This argument is mainly for internal use within the program code for scheduling tasks to clear shimcache upon system reboot.

Values should come after their flags, separated by spaces.

### Examples
#### Open a file with notepad and remove traces after notepad is closed
```
artifact-exterminator.exe -f notepad.exe --args C:\Windows\win.ini
```

#### Open a file with notepad and remove traces after notepad is closed, and specified kill switch is activated

```
artifact-exterminator.exe -f notepad.exe --args C:\Windows\win.ini --killswitch-ip 127.0.0.1 --killswitch-port 8080 --killswitch-poll 3
```
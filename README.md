## Project description

Anti-forensic tool that is able to remove traces left by a program in the following areas:
- Registry
- ShimCache
- Event Viewer
- MFT (requires further exploration)

The tool aims to be highly configurable yet easy for an attacker to use.
Configurable fields:

| Field | Options |
| ----- | ------- |
| When to clean up | After a set amount of time |
| | After the malware terminates
| | After a kill switch has been activated (the kill switch URL can be specified dynamically when starting the program)
| | After the program receives a message (easier to avoid detection, but must make sure that the device is contactable by other remote devices)
| What to clean up | Registry (revert to previous state) |
| | Event viewer
| | Shimcache


### Example scenario
The attacker launches our program before launching his own attacks.

Our program will have options to configure with regards to conducting cleanup after the malware is launched.

Example:

Attacker runs our program, specifying the following actions:

All traces of file “destroyer.exe” to be wiped out after the process has ended

All traces of file “persistence.exe” to be wiped out after half a day (<- Can consider a more dynamic option, for example, to wipe out after the program receives a HTTP request remotely from the attacker. Something like a kill switch)

Attacker runs malware destroyer.exe through our program (so our program and keep track of the process)

Destroyer.exe drops another executable “persistence.exe”

Destroyer.exe sets up persistence by adding registry key

Our program runs persistence.exe in the background

Specified time limit is reached, all traces left by persistence.exe is wiped out

Registry state is reverted back to original state

MFT records related to persistence.exe deleted

ShimCache records of commands related persistence.exe deleted
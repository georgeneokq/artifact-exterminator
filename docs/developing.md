# Developer guide

Warning!!! Changes may be destructive and should be tested on a Windows virtual machine.

Self-explanatory features such as registry backup and restoration will not be documented.

## Testing features

### Kill switch

Run the following command:
`.\artifact-exterminator.exe -f calc.exe --killswitch-ip 127.0.0.1 --killswitch-port 8080 --killswitch-poll 3`

This will run calc.exe, and start polling for the kill switch hosted on the local machine on port 8080,
with an interval of 3 seconds once the calc.exe app is closed.

On another terminal, run the [example kill switch python script](external/sock.py). The script will host a socket which serves as the kill switch, and will be activated after 6 seconds.
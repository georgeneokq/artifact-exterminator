import winreg
import re
import os
import sys

WIN10_HEADER = b'\x31\x30\x74\x73'

def shimcache_remove(executable_name):
    """Removes Shimcache entry of the specified executable"""
    local_registry = winreg.ConnectRegistry(None, winreg.HKEY_LOCAL_MACHINE)
    shim_key = winreg.OpenKeyEx(local_registry,r'SYSTEM\CurrentControlSet\Control\Session Manager\AppCompatCache',0,winreg.KEY_ALL_ACCESS)
    shim_value = winreg.QueryValueEx(shim_key, 'AppCompatCache')[0] #QueryValueEx returns a tuple and the key value is store in Index 0

    #Find all entry locations
    #Shimcache entries are seperated by their header 10ts or b'\x31\x30\x74\x73'
    entry_index = [i.span()[0] for i in re.finditer(WIN10_HEADER, shim_value)]
    print(entry_index)
    search_string = "".join([r"\x00" + i for i in executable_name]) #Making it into RE searchable format
    search_string = search_string.encode().decode('unicode_escape').encode("raw_unicode_escape")

    num_entries = len([i.span()[0] for i in re.finditer(search_string, shim_value)])
    print("DEBUG: Number of entries = " + str(num_entries))

    for i in range(num_entries):
        executable_index = re.search(search_string, shim_value).span()[1]
        print("Executable found at " + str(executable_index))
        for k in enumerate(entry_index):
            if k[1] > executable_index:
                SIndex = k[0] - 1
                FIndex = k[0]
                break
        
        print("Indexes Found: Executable found between " + str(entry_index[SIndex]), str(entry_index[FIndex]))
        
        shim_value = shim_value[:entry_index[SIndex]] + shim_value[entry_index[FIndex]:]
        entry_index = [i.span()[0] for i in re.finditer(WIN10_HEADER, shim_value)]
        print("Purged!")

            
    print(len([i.span()[0] for i in re.finditer(search_string, shim_value)]))

    #Set the Registry key to new Shim Value
    winreg.SetValueEx(shim_key,'AppCompatCache',0,winreg.REG_BINARY,shim_value)

        
            


    
        
        

shimcache_remove('vcredist_x86.exe')
import winreg
import os
import sys


def amcache_remove(executable_name):
    """Deletes the associated Amcache entry"""
    os.system(r'cmd /k REG LOAD HKLM\AM C:\Windows\appcompat\Programs\Amcache.hve')
    
    local_registry = winreg.ConnectRegistry(None, winreg.HKEY_LOCAL_MACHINE)
    amcache_key = winreg.OpenKey(local_registry,r'AM\Root\InventoryApplicationFile',0,winreg.KEY_ALL_ACCESS)
    subkey_count = winreg.QueryInfoKey(amcache_key)[0] #Get the amount of subkeys
    
    #Amcache Max Length of EXE name in Keys is 16 and in all lowercase
    if len(executable_name) > 16:
        executable_name = executable_name[:16].lower()
        print(executable_name)
    
    for i in range(subkey_count): #enumerates through the subkeys
        Subkey = winreg.EnumKey(amcache_key,i)
        if executable_name in Subkey:
            print("FOUND!")
            winreg.DeleteKey(amcache_key, Subkey) # should probably try except kek
            print("Deleted Key")
            break
        
amcache_remove('AppCompatCacheParser.exe')
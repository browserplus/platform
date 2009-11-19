# Just prints the registry entries for b+
#
require 'fileutils'
require 'rbconfig'
include Config

require 'win32/registry'

# Find all CLSID entries with an InProcServer32 of YBPAddon*
Win32::Registry::HKEY_CLASSES_ROOT.open('CLSID', Win32::Registry::KEY_ALL_ACCESS) do |reg|
    reg.keys.each do |k|
        reg1 = Win32::Registry::HKEY_CLASSES_ROOT.open("CLSID\\#{k}")
        reg1.each_key do |k2, wtime| 
            if (k2 == "InprocServer32")
                reg2 = Win32::Registry::HKEY_CLASSES_ROOT.open("CLSID\\#{k}\\#{k2}")
                reg2.each do |subkey, type, data|
                    if (data =~ /YBPAddon/)
                        puts("HKEY_CLASSES_ROOT\\CLSID\\#{k}")
                        break
                    end
                end
            end
        end
    end
end

# Find all reference count entries for our plugins 
# In versions prior to 1.1.0, plugins were reference counted.
Win32::Registry::HKEY_LOCAL_MACHINE.open('SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls') do |reg|
    reg.each do |subkey, type, data|
        if ((subkey =~ /\\Internet Explorer\\Plugins\\YBPAddon/) \
            || (subkey =~ /Internet Explorer\\Plugins\\nspr4.dll/) \
            || (subkey =~ /Mozilla Firefox\\plugins\\npybrowserplus/) \
            || (subkey =~ /Mozilla Firefox\\plugins\\nspr4.dll/))
            puts "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls\\#{subkey}"
        end
    end
end

# Find AppID entries with YBPAddon
Win32::Registry::HKEY_CLASSES_ROOT.open('AppId', Win32::Registry::KEY_ALL_ACCESS) do |reg|
    reg.keys.each do |k|
        if (k =~ /YBPAddon/) 
            puts("HKEY_CLASSES_ROOT\\AppId\\#{k}")
        end            
        reg1 = Win32::Registry::HKEY_CLASSES_ROOT.open("AppId\\#{k}")
        reg1.each do |subkey, type, data|
            if (data =~ /YBPAddon/)
                puts("HKEY_CLASSES_ROOT\\AppId\\#{k}")
            end
        end
    end
end

# Find all Typelib entries with YBPAddon* 
Win32::Registry::HKEY_CLASSES_ROOT.open('TypeLib', Win32::Registry::KEY_ALL_ACCESS) do |reg|
    reg.keys.each do |k|
        reg1 = Win32::Registry::HKEY_CLASSES_ROOT.open("TypeLib\\#{k}")
        reg1.each_key do |k2, wtime| 
            reg2 = Win32::Registry::HKEY_CLASSES_ROOT.open("TypeLib\\#{k}\\#{k2}")
            reg2.each do |subkey, type, data|
                if (data =~ /YBPAddon/)
                    puts("HKEY_CLASSES_ROOT\\TypeLib\\#{k}")
                    break
                end
            end
        end
    end
end

# Find all entries for Yahoo.BPCtl* 
Win32::Registry::HKEY_CLASSES_ROOT.open('', Win32::Registry::KEY_ALL_ACCESS) do |reg|
    reg.keys.each do |k|
        if (k =~ /Yahoo.BPCtl/) 
            puts("HKEY_CLASSES_ROOT\\#{k}")
        end            
    end
end

# Find all entries for mimetype *yahoo-browserplus*
Win32::Registry::HKEY_CLASSES_ROOT.open('MIME\\Database\\Content Type', Win32::Registry::KEY_ALL_ACCESS) do |reg|
    reg.keys.each do |k|
        if (k =~ /yahoo-browserplus/) 
            puts("HKEY_CLASSES_ROOT\\MIME\\Database\\Content Type\\#{k}")
        end            
    end
end


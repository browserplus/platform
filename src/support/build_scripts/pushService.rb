#!/usr/bin/env ruby

# Push a corelet to distro server
# Must be run from corelets build dir

require 'rbconfig'
include Config

def usage()
    puts "Usage #{File.basename($0)} [-plat=ind] [-url=distServerUrl] [-password=pwd] corelet..."
    exit 1
end

usage() if ARGV.length < 1 || ARGV[0] == "help"

platform = nil
url = "http://bp-dev.corp.yahoo.com/api"
passwordArg = "-password FreeYourBrowser"
certType = "devel"

# parse command line
coreletStart = 0
for i in 0..ARGV.length-1
    arg = ARGV[i]
    v = arg.split('=')
    if v.length == 2
       case v[0]
        when '-url'
            url = v[1]
        when '-plat'
            usage() if v[1] != "ind"
            platform = v[1]
        when "-password"
            passwordArg = "-password " + v[1]
        else
            usage()
        end
    else 
        coreletStart = i
        break
    end
end

usage() if certType == nil
for i in coreletStart..ARGV.length-1
    corelet = ARGV[i]
    if !platform
        if CONFIG['arch'] =~ /mswin/
            platform = "win32"
        else
            platform = "osx"
        end
    end

    exeSuffix = ""
    if CONFIG['arch'] =~ /mswin/
        exeSuffix = ".exe"
    end


    topDir = File.dirname(File.expand_path(__FILE__))
    coreletDir = File.join(corelet, "Main", ARGV[i])
    coreletDir = File.directory?(coreletDir) ? coreletDir : ARGV[i]
    srcDir = File.join(topDir, "..", "..")
    publisher = File.join(srcDir, "build", "bpsdk_internal", 
                          "bin", "ServicePublisher#{exeSuffix}")
    keyDir = File.join(srcDir, "support", "signing", "#{certType}_certs")
    privateKey= File.join(keyDir, "BrowserPlus.pvk")
    publicKey= File.join(keyDir, "BrowserPlus.crt")
  
    if !File.executable? publisher
        $stderr.puts "Can't run publisher (build required?): #{publisher}"
        exit 1
    end

    cmd = "#{publisher} -v -privateKey #{privateKey} -publicKey #{publicKey} "
    cmd += "#{passwordArg} -p #{platform} #{coreletDir} #{url}"
    if (!system(cmd) || $? != 0)
        $stderr.puts "Error executing command: #{cmd}"
    end
end


#!/usr/bin/env ruby

# a mac specific ruby script that builds a distributable dmg image of
# our osx installer.  This script also takes binaries and scripts
# present in the installer/ dir and turns them into a nice .app bundle that
# the end user can just click on.

require 'fileutils'
require 'pathname'

# process some arguments
if ARGV[0] =~ /help/ 
  puts "Usage: ruby -s #{$0} [-outDir=<path>] [-fname=<name of outputted binary>]"
  puts "                     [-verbose=true]"
  exit 1
end

puts "outDir: #{$outDir}"

#default argument values
$fname ||= "BrowserPlusInstaller"
$outDir ||= File.dirname(File.expand_path(__FILE__))
instDir = File.dirname(File.expand_path(__FILE__))

puts "outDir: #{$outDir}"

# does outdir exist?
if !File.directory? $outDir
  puts "fail: cannot write to output directory: #{$outDir}"
  exit 1
end

# now we're ready to create the application directory and it's required
# substructure
appDir = File.join($outDir, "#{$fname}.app")
FileUtils.rm_rf(appDir)
FileUtils.mkdir(appDir, :mode => 0755, :verbose => $verbose)

dmgFile = File.join($outDir, "#{$fname}.dmg")
FileUtils.rm_rf(dmgFile)

contentsDir = File.join(appDir, "Contents")
FileUtils.mkdir_p(contentsDir, :mode => 0755, :verbose => $verbose)

binDir = File.join(contentsDir, "MacOS")
FileUtils.mkdir_p(binDir, :mode => 0755, :verbose => $verbose)

resourcesDir = File.join(contentsDir, "Resources")
FileUtils.mkdir_p(resourcesDir, :mode => 0755, :verbose => $verbose)

# how about a PkgInfo file
File.open(File.join(contentsDir, "PkgInfo"), "w") { |x|
  x.write "AAPL????"
} 

# now an Info.plist
infoPlistData = <<ENDPLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleDisplayName</key>
	<string>BrowserPlusInstaller</string>
	<key>CFBundleExecutable</key>
	<string>BrowserPlusInstaller</string>
	<key>CFBundleIdentifier</key>
	<string>com.yahoo.BrowserPlusInstaller</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>BrowserPlusInstaller</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
    <key>CFBundleIconFile</key>
    <string>bplus_icon.icns</string>
	<key>NSPrincipalClass</key>
	<string>NSApplication</string>
</dict>
</plist>
ENDPLIST

File.open(File.join(contentsDir, "Info.plist"), "w") { |x|
  x.write infoPlistData
} 

# now let's copy everything into the binary dir of the bundle.  not
# exactly a correctly organized bundle, but who's counting?
Dir.glob(File.join(instDir, "**", "*")) do |x|
  if (File.file?(x) && x !~ /~$/ && x !~ /\/\.svn\// &&
      x !~ /.app$/ && x !~ /.dmg$/ && x !~ /.rb$/ && x !~ /.sparseimage$/ &&
      x !~ /.icns$/) then
    to = Pathname.new(x).relative_path_from(Pathname.new(instDir))
    to = File.join(binDir, to)
    FileUtils.mkdir_p(File.dirname(to), :mode => 0755,
                      :verbose => $verbose)
    FileUtils.install(x, to, :mode => 0755, :verbose => $verbose)
  end
end

# now let's copy in a nice icon
FileUtils.install(File.join(instDir, "bplus_icon.icns"), resourcesDir,
                  :mode => 0644, :verbose => $verbose)

# make a a dmg with a pretty installer background.  
# from http://digital-sushi.org/entry/how-to-create-a-disk-image-installer-for-apple-mac-os-x/
# TODO: do we want version number appended to filename here?
sparseImagePath = File.join($outDir, "#{$fname}.sparseimage")
templatePath = File.join($outDir, "#{$fname}.sparseimage")
mountPoint = "/tmp/XXXBrowserPlusInstallerMountPoint"

FileUtils.rm_f(sparseImagePath, :verbose => $verbose)
system("hdiutil convert #{instDir}/BrowserPlusInstaller.template.dmg -format UDSP -o #{sparseImagePath}")
system("hdiutil mount -private #{sparseImagePath} -mountpoint #{mountPoint}")
system("cp -R #{appDir}/ \"#{mountPoint}/BrowserPlus Installer.app\"")
system("hdiutil eject \"#{mountPoint}\"")
system("hdiutil convert #{sparseImagePath} -format UDBZ -o #{dmgFile}")
# with a custom .app installer, this is no longer useful (YIB-2540072)
# system("hdiutil internet-enable #{dmgFile}")

# clean up!
FileUtils.rm_rf(appDir, :verbose => $verbose)
FileUtils.rm_rf(sparseImagePath, :verbose => $verbose)

puts "Wrote dmg to #{dmgFile}"

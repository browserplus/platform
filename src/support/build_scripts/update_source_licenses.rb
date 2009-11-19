#!/usr/bin/env ruby

#
# a script to iterate recursively through a directory structure and
# update files in place with correct license headers
# 

require 'pathname'

$pathToLicense =
  File.join(File.dirname(__FILE__), "..", "..", "..", "COPYING")
$licenseContent = File.new($pathToLicense).read

LICENSE_BEGIN_MARK = "***** BEGIN LICENSE BLOCK *****"
LICENSE_END_MARK = "***** END LICENSE BLOCK *****"

def addInLicense(fname, lineBeginComment, commentStart, commentEnd)
  contents = File.new(fname).read
  # generate a regex that would match a license header
  regex = ""
  regex += Regexp.quote(commentStart) + "\\s*" if commentStart
  regex += Regexp.quote(lineBeginComment) if lineBeginComment
  regex += Regexp.quote(LICENSE_BEGIN_MARK) + "\\s*"
  regex += ".*"
  regex += Regexp.quote(lineBeginComment) if lineBeginComment
  regex += Regexp.quote(LICENSE_END_MARK) + "\\s*" 
  regex += Regexp.quote(commentEnd) + "\\s" if commentEnd
  r = Regexp.new(regex, Regexp::MULTILINE)
  contents = File.new(fname).read
  m = r.match(contents)
  puts (m ? "MATCH" : "NO MATCH") + " " + fname

  # now let's generate a well formed license header
  hdr = ""
  hdr += commentStart + "\n" if commentStart
  hdr += lineBeginComment + LICENSE_BEGIN_MARK + "\n"
  $licenseContent.each_line { |l|
    hdr += lineBeginComment + l.chomp + "\n"
  }
  hdr += lineBeginComment + LICENSE_END_MARK + "\n" 
  hdr += commentEnd + "\n" if commentEnd

  # sub in or append
  if (m) 
    contents.sub!(r, hdr)
  else
    newContents = hdr + "\n" + contents
    contents = newContents
  end
  File.open(fname,  "w+") { |f| f.write(contents) }
end

Dir["**/*"].each do |fname|
  next if !File.file? fname
  next if fname =~ /\.svn/ 
  next if fname =~ /\.git/ 
  next if fname =~ /^build\// 

  # we must handle the following types of files:
  # .mm files - objective C with c++ style comments
  # CMakeLists.txt - cmake files with hash (#) style comments
  # .cpp - c++ files
  # .h - c/c++ files
  # 
  # NOTE: any of the above files may have a .erb or .cmakeIn suffix appended
  #       to the actual file type
  
  m = fname.match(/\.([a-z]+)(?:\.cmakeIn)?(?:\.erb)?$/)
  next if m == nil
  ext = m[1]

  if ext == 'cpp' || ext == 'cc' || ext == 'h' || ext == 'hh' || ext == 'mm'
    addInLicense(fname, ' * ', '/**', ' */')
  elsif fname =~ /CMakeLists\.txt/ || ext == "cmake"
    addInLicense(fname, '# ', nil, nil)    
  end
end

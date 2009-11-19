#!/usr/bin/env ruby -s
#
# A script to validate strings.json and report statistics about needed.
# translations
#
# 1. parse strings.json right next to this script
# 2. rip through all locales and nested keys, building up a complete
#    set of nested hashes
# 3. 

require "pp"

$:.push(File.dirname(File.expand_path(__FILE__)))
require "json/json.rb"

# 1 parse strings.json
stringsJson = File.join(File.dirname(File.expand_path(__FILE__)),
                        "strings.json")
throw "missing strings.json: #{stringsJson}" if !File.file? stringsJson
fullData = JSON.parse(File.read(stringsJson))

# 2. rip through all locales and nested keys, building up a complete
#    set of nested hashes
complete = Hash.new
fullData.each { |locale, data|
  data.each { |toplevel,middle|
    node = nil
    if complete.has_key? toplevel
      node = complete[toplevel]
    else
      node = Hash.new
    end
    middle.each {|k,v| node[k] = "XXX" }
    complete[toplevel] = node
  }
}

# now complete is a full set of keys required for each locale
# let's go through locale by locale and verify everything is complete
allGood = true
damagedLocales = 0
totalMissingTranslations = 0

fullData.each { |locale, data|
  missingKeys = 0
  missingTranslations = 0

  complete.each { |toplevel,middle|
    node = nil

    if data.has_key? toplevel
      node = data[toplevel]
    else
      node = Hash.new
    end
    middle.each {|k,v|
      if !node.has_key? k
        missingKeys += 1 
      elsif node[k] == 'XXX'
        missingTranslations += 1
      end
    }
  }

  if (missingKeys > 0)
    puts "#{locale} is invalid, missing #{missingKeys} keys" 
    damagedLocales += 1 
  end
  if (missingTranslations > 0)
    puts "#{locale} is missing #{missingTranslations} translations" 
    totalMissingTranslations += missingTranslations
  end
}

returnValue = 0

if (damagedLocales > 0)
  returnValue = 1
  puts "WARNING: #{damagedLocales} locales are damaged"
end

if (totalMissingTranslations > 0)
  puts "WARNING: #{totalMissingTranslations} translations are needed"
end

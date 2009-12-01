#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"
require 'rbconfig'
include Config

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_dist_production
    pwd = Dir.pwd
    x=false
    output=""
    x, output = runCMake
    assert_equal(x, true, "cmake failed: >>>\n" + output + "<<<\n");
    x, output = doBuild
    assert_equal(x, true, "build failed: >>>\n" + output + "<<<\n");

    # now we've got a built tree.  let's assert that the files
    # we expect to be there are actually there 
    if CONFIG["arch"] =~ /win32/i  
      expected = Set.new [
                          "work/dist/bin/debug/getquote.exe",
                          "work/dist/include/YStockQuote/YStockQuoteExports.h",
                          "work/dist/include/YStockQuote/YStockQuote.h",
                          "work/dist/include/YStockQuote/quote.h",
                          "work/dist/lib/debug/YStockQuote.lib",
                          "work/dist/bin/debug/YStockQuote.dll",
                          "work/dist/lib/debug/YStockQuote_s.lib"
                         ];

    else
      expected = Set.new [
                          "work/dist/bin/getquote",
                          "work/dist/include/YStockQuote/YStockQuoteExports.h",
                          "work/dist/include/YStockQuote/YStockQuote.h",
                          "work/dist/include/YStockQuote/quote.h",
                          "work/dist/lib/libYStockQuote_s.a"
                         ];
    end

    actual = Set.new
    Dir.glob(File.join("work", "dist", "**", "*")).each { |x| actual.add(x) }
    
    assert(expected.subset?(actual),
           "expected set:\n\n#{expected.inspect}\n\n" +
           "not in actual set\n\n#{actual.inspect}\n\n")

    Dir.chdir(pwd)
    cleanUp
  end
end

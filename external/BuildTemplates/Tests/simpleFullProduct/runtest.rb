#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"
require 'rbconfig'
include Config

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_dist_production
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
                          "work/dist/samples/sampleStockQuoteFetcher/getquote.cpp",
                          "work/dist/samples/sampleStockQuoteFetcher/CMakeLists.txt",
                          "work/dist/include/YStockQuote/YStockQuoteExports.h",
                          "work/dist/include/YStockQuote/YStockQuote.h",
                          "work/dist/include/YStockQuote/quote.h"
                         ];

    else
      expected = Set.new [
                          "work/dist/bin/getquote",
                          "work/dist/samples/sampleStockQuoteFetcher/getquote.cpp",
                          "work/dist/samples/sampleStockQuoteFetcher/CMakeLists.txt",
                          "work/dist/include/YStockQuote/YStockQuoteExports.h",
                          "work/dist/include/YStockQuote/YStockQuote.h",
                          "work/dist/include/YStockQuote/quote.h"
                         ];
    end

    actual = Set.new
    Dir.glob(File.join("work", "dist", "**", "*")).each { |x| actual.add(x) }
    
    assert(expected.subset?(actual),
           "expected set:\n\n#{expected.inspect}\n\n" +
           "not in actual set\n\n#{actual.inspect}\n\n")

    cleanUp
  end
end

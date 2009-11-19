#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"
require 'rbconfig'
include Config

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_nested_projects
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
      expected =
        Set.new [
                 "work/depender/dist/bin/debug/dependerBinaryThatLinksALibFromDependee.exe",
                 "work/dependee/dist/bin/debug/getquote.exe",
                 "work/dependee/dist/samples/sampleStockQuoteFetcher/CMakeLists.txt",
                 "work/dependee/dist/samples/sampleStockQuoteFetcher/getquote.cpp"
                ];
    else
      expected =
        Set.new [
                 "work/depender/dist/bin/dependerBinaryThatLinksALibFromDependee",
                 "work/dependee/dist/bin/getquote",
                 "work/dependee/dist/samples/sampleStockQuoteFetcher/CMakeLists.txt",
                 "work/dependee/dist/samples/sampleStockQuoteFetcher/getquote.cpp"
                ];
    end
    actual = Set.new
    Dir.glob(File.join("work", "dependee", "dist", "**", "*")).each do |x|
      actual.add(x)
    end
    Dir.glob(File.join("work", "depender", "dist", "**", "*")).each do |x|
      actual.add(x)
    end

    assert(expected.subset?(actual),
           "expected set:\n\n#{expected.inspect}\n\n" +
           "not in actual set\n\n#{actual.inspect}\n\n")
    Dir.chdir(pwd)
    cleanUp
  end
end

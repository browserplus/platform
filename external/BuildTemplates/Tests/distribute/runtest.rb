#!/usr/bin/env ruby

require 'set'
require 'test/unit'
require "../ybttesttools"

class SimpleFullProductTest < Test::Unit::TestCase
  include YBTTestTools
  
  def test_dist_production
    x=false
    output=""
    x, output = runCMake
    assert_equal(x, true, "cmake failed: >>>\n" + output + "<<<\n");

    # now we've got a built tree.  let's assert that the files
    # we expect to be there are actually there 
    expected = Set.new [
                        "work/dist/share/foo/afile.txt",
                        "work/dist/share/foo/bfile.txt",
                        "work/dist/share/some/other/path/afile.txt",
                        "work/dist/share/some/path/afile.txt"
                       ];
    actual = Set.new
    Dir.glob(File.join("work", "dist", "**", "*")).each { |x| actual.add(x) }
    
    assert(expected.subset?(actual))

    cleanUp
  end
end

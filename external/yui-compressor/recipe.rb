{
  :configure => lambda { |c|
    # XXX: check to see if java is installed, halt build if not
  }, 
  :post_install_common => lambda { |c|
    tgtDir = File.join(c[:output_dir], "share", "jars")
    FileUtils.mkdir_p(tgtDir)
    FileUtils.cp(File.join(c[:src_dir], "yuicompressor-2.4.jar"),
                 tgtDir, :verbose => true)
  }
}

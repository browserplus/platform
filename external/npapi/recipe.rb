{
  :post_install_common => lambda { |c|
    Dir.glob(File.join(c[:src_dir], "headers", "*")).each { |f|
      FileUtils.cp_r f, c[:output_inc_dir]
    }
  }
}

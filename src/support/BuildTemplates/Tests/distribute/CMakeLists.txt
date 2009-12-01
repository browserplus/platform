# Test YBT_DISTRIBUTE_FILE

# include build templates
SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

YBT_DISTRIBUTE_FILES(share/foo some/path/afile.txt some/path/bfile.txt)
YBT_DISTRIBUTE_FILES(share PRESERVE
                     some/other/path/afile.txt
                     some/path/afile.txt)


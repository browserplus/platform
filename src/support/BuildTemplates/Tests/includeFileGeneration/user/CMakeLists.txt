PROJECT(BazTheUser)

#use build templates
SET(YBT_PATH ../../..)
INCLUDE("${YBT_PATH}/BuildTemplates.cmake")

#setup include and link paths from the generated libs from useee
INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/../useee/work/include.cmake")

# all possible linking permutations
ADD_SUBDIRECTORY(static)
ADD_SUBDIRECTORY(shared)
IF (UNIX)
ADD_SUBDIRECTORY(sharedfoo)
ADD_SUBDIRECTORY(sharedbar)
ENDIF ()

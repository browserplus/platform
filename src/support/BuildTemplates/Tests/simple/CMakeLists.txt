# A test building a simple library (XXX test, binary, etc?)
# XXX: actually do the build and check the results?

# include build templates
SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

YBT_BUILD(LIBRARY simple)

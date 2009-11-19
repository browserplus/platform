# include build templates
SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

# now let's try to build a bogus object
YBT_BUILD(FOOBAR bogus)

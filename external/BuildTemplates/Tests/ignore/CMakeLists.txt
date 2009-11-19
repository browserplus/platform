# A test of explicit source specification

# include build templates
SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

# implicitly define our sources ignoring files containing "bad" 
SET (ignore_IGNORE_PATTERNS ".*bad.*")
YBT_BUILD(LIBRARY ignore)


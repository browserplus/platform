# A test of explicit source specification

# include build templates
SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

# explicitly define our sources
SET(explicit_SOURCES foo.cpp)
SET(explicit_PRIVATE_HEADERS foo.h)
SET(explicit_PUBLIC_HEADERS public/bar.h)

YBT_BUILD(LIBRARY explicit)

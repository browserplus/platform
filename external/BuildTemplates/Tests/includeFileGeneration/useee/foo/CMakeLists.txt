SET(YBT_PATH ../../../..)
INCLUDE("${YBT_PATH}/BuildTemplates.cmake")

PROJECT(Foo)
SET(Foo_REQUIRES Bar)
YBT_BUILD(LIBRARY Foo)

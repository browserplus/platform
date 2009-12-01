SET(YBT_PATH ../../../..)
INCLUDE("${YBT_PATH}/BuildTemplates.cmake")

PROJECT(Bar)
YBT_BUILD(LIBRARY Bar)

#############################################################
# a sample cmake file for our "deep" sample
#############################################################

SET(YBT_PATH ../..)
INCLUDE(${YBT_PATH}/BuildTemplates.cmake)

YBT_BUILD(SAMPLE deepSample)

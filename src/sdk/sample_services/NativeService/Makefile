# A minimal makefile to build the sample native service
# environment inputs:
#  BPSDK_PATH: the path to the BrowserPlus SDK

BPSDK_PATH ?= ../..
SRC_PATH ?= .
INCLUDE = -I${BPSDK_PATH}/include
NAME = SampleService
PLAT = $(strip $(shell uname))
ifeq ($(PLAT), Linux)
  DYFLAG=-rdynamic
else
  DYFLAG=-bundle
endif

${NAME}/lib${NAME}.so: ${SRC_PATH}/service.cpp ${NAME} ${NAME}/manifest.json
	g++ $(DYFLAG) -o $@ ${INCLUDE} $<

${NAME}/manifest.json: ${SRC_PATH}/manifest.json
	sed -e 's/@ServiceLibrary@/lib${NAME}.so/' < $< > $@

${NAME}:
	mkdir -p ${NAME}

.PHONY: clean
clean:
	rm -rf ${NAME} *~

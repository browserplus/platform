# A minimal makefile to build the sample native service
# environment inputs:
#  BPSDK_PATH: the path to the BrowserPlus SDK

BPSDK_PATH ?= ../..
SRC_PATH ?= .
INCLUDE = -I${BPSDK_PATH}/include
NAME = SampleService
PLAT = $(strip $(shell uname))
ifeq ($(PLAT), Linux)
  PLAT_FLAGS=-shared -fPIC
else
  PLAT_FLAGS=-bundle -m32
endif

${NAME}/lib${NAME}.so: ${SRC_PATH}/service.cpp ${NAME} ${NAME}/manifest.json
	g++ $(PLAT_FLAGS) -g -o $@ ${INCLUDE} $<

${NAME}/manifest.json: ${SRC_PATH}/manifest.json
	sed -e 's/@ServiceLibrary@/lib${NAME}.so/' < $< > $@

${NAME}:
	mkdir -p ${NAME}

.PHONY: clean
clean:
	rm -rf ${NAME} *~

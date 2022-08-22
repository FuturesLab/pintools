# Set accordingly

CC       ?= gcc 
CXX      ?= g++
ARCH     ?= intel64
MAKE_DIR ?= $(shell pwd)
PIN_ROOT ?= pin-3.16

ifeq ($(wildcard $(PIN_ROOT)),)
    $(shell tar -xf $(PIN_ROOT).tar.gz $(PIN_ROOT))
endif

ifneq ("$(PIN_ROOT)", "")
	CONFIG_ROOT := $(PIN_ROOT)/source/tools/Config
	include $(CONFIG_ROOT)/makefile.config
    include $(TOOLS_ROOT)/Config/makefile.default.rules
endif

.PHONY: directories

all: objdirs analyze ftrace loopcov

objdirs: obj-${ARCH}

analyze: objdirs
	$(MAKE) TARGET=$(ARCH) obj-$(ARCH)/pinAnalyze.so -w
	echo "#!/bin/sh\n$(MAKE_DIR)/$(PIN_ROOT)/pin -t $(MAKE_DIR)/obj-$(ARCH)/pinAnalyze.so -- "$$\@"" > pinAnalyze
	chmod 755 pinAnalyze

ftrace: objdirs
	$(MAKE) TARGET=$(ARCH) obj-$(ARCH)/pinFTrace.so -w
	echo "#!/bin/sh\n$(MAKE_DIR)/$(PIN_ROOT)/pin -t $(MAKE_DIR)/obj-$(ARCH)/pinFTrace.so -- "$$\@"" > pinFTrace
	chmod 755 pinFTrace

bbtrace: objdirs
	$(MAKE) TARGET=$(ARCH) obj-$(ARCH)/pinBBTrace.so -w
	echo "#!/bin/sh\n$(MAKE_DIR)/$(PIN_ROOT)/pin -t $(MAKE_DIR)/obj-$(ARCH)/pinBBTrace.so -- "$$\@"" > pinBBTrace
	chmod 755 pinBBTrace

loopcov: objdirs
	$(MAKE) TARGET=$(ARCH) obj-$(ARCH)/pinLoopCov.so 
	echo "#!/bin/sh\n$(MAKE_DIR)/$(PIN_ROOT)/pin -t $(MAKE_DIR)/obj-$(ARCH)/pinLoopCov.so -- "$$\@"" > pinLoopCov
	chmod 755 pinLoopCov

install: all 
	sudo cp pinAnalyze /usr/local/bin/pinAnalyze
	sudo cp pinFTrace /usr/local/bin/pinFTrace
	sudo cp pinLoopCov /usr/local/bin/pinLoopCov

clean:
	rm -rf obj-*
	rm -rf pinAnalyze pinFTrace pinLoopCov
	rm -rf *.log
	
clean-pin:
	rm -rf $(PIN_ROOT)

LIBXIVELY := $(shell git rev-parse --show-toplevel)
include $(LIBXIVELY)/Makefile.include

XI_SOURCES = $(wildcard *.c)

XI_LAYER_DIRS := io/$(XI_IO_LAYER)

ifeq ($(XI_NOB_ENABLED),true)
    XI_LAYER_DIRS += nob
endif

ifeq ($(XI_MQTT_ENABLED),true)
    XI_LAYER_DIRS += mqtt/layer mqtt/codec
else
    XI_LAYER_DIRS += csv http
endif

XI_INCLUDE_DIRS := ./ $(XI_LAYER_DIRS)

ifeq ($(MAKECMDGOALS),flags)
    XI_CFLAGS += $(foreach layerdir,$(XI_INCLUDE_DIRS),-I$(realpath ./$(layerdir)))
else
    XI_CFLAGS += $(foreach layerdir,$(XI_INCLUDE_DIRS),-I./$(layerdir))
endif

XI_SOURCES += $(foreach layerdir,$(XI_LAYER_DIRS),\
	$(wildcard $(layerdir)/*.c $(layerdir)/*.cpp))

all: $(XI)

objs: $(XI_OBJS)
deps: $(XI_DEPS)

$(XI): $(XI_OBJS)
	$(AR) $(XI_ARFLAGS) $(XI) $(XI_OBJS)

include $(LIBXIVELY)/Makefile.rules

ifneq ($(MAKECMDGOALS),clean)
  -include $(XI_DEPS)
endif

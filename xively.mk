# Dir declaration
MID_XIVELY_DIR = $(MID_DIR)/xively

XI_BUILD_TYPE ?= release
#XI_BUILD_TYPE ?= debug
XI_DEBUG_ASSERT ?= 1
XI_IO_LAYER ?= posix
XI_NOB_ENABLED ?= false
XI_IO_LAYER_POSIX_COMPAT ?= 1
XI_DEBUG_PRINTF ?= xprintf
XI_ALLOCATOR_USE ?= lwip
XI_USER_AGENT ?= '"libxively-$(XI_IO_LAYER)"'

include $(MID_XIVELY_DIR)/Makefile.include


# Dirs need to be compiled
MID_XIVELY_ASMSRCDIR	=
MID_XIVELY_CSRCDIR	= $(MID_XIVELY_DIR)/src/libxively \
				$(MID_XIVELY_DIR)/src/libxively/io/$(XI_IO_LAYER)

MID_XIVELY_INCDIR	= $(MID_XIVELY_DIR)/src/libxively \
				$(MID_XIVELY_DIR)/src/libxively/io/$(XI_IO_LAYER)

ifeq ($(XI_NOB_ENABLED),true)
MID_XIVELY_CSRCDIR	+= $(MID_XIVELY_DIR)/src/libxively/nob
MID_XIVELY_INCDIR	+= $(MID_XIVELY_DIR)/src/libxively/nob
endif

MID_INCDIRS += $(MID_XIVELY_INCDIR)
MID_CSRCDIRS += $(MID_XIVELY_CSRCDIR)
MID_ASMSRCDIRS += $(MID_XIVELY_ASMSRCDIR)

# dir declaration
MID_XIVELY_C_API_DIR = $(EMBARC_ROOT)/middleware/xively

# Middleware Definitions
ifndef XI_USER_CONFIG

ifeq ($(XI_IO_LAYER),posix)
  XI_DEFINES += -DXI_IO_LAYER=0
endif

ifeq ($(XI_IO_LAYER),contiki)
  XI_DEFINES += -DXI_IO_LAYER=4
endif

  XI_DEFINES += $(foreach constant,$(XI_CONFIG),-D$(constant))
  XI_DEFINES += -DXI_USER_AGENT=$(XI_USER_AGENT)
  XI_DEFINES += -DXI_IO_LAYER_POSIX_COMPAT=$(XI_IO_LAYER_POSIX_COMPAT)
ifeq ($(XI_ALLOCATOR_USE), lwip)
  XI_DEFINES += -DXI_ALLOCATOR=2
endif
else
  $(warning "Using XI_USER_CONFIG instead of: '$(XI_CONFIG)'")
  XI_DEFINES += -DXI_USER_CONFIG=1
  ifdef XI_USER_CONFIG_PATH
    $(info "Will include '$(XI_USER_CONFIG_PATH)/xi_user_config.h'")
    XI_DEFINES += -I$(XI_USER_CONFIG_PATH)
  endif
endif

# Middleware Definitions
MID_XIVELY_DEFINES = -DMID_XIVELY $(XI_DEFINES)
MID_DEFINES += $(MID_XIVELY_DEFINES)
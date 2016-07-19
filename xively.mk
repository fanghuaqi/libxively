# dir declaration
MID_XIVELY_C_API_DIR = $(MIDDLEWARES_ROOT)/xively

XI_BUILD_TYPE ?= release
#XI_BUILD_TYPE ?= debug
XI_DEBUG_ASSERT ?= 1
XI_IO_LAYER ?= posix
XI_NOB_ENABLED ?= false
XI_IO_LAYER_POSIX_COMPAT ?= 1
XI_DEBUG_PRINTF ?= xprintf
XI_ALLOCATOR_USE ?= lwip
XI_USER_AGENT ?= \"libxively-$(XI_IO_LAYER)\"

## include xively makefile config
include $(MID_XIVELY_C_API_DIR)/Makefile.include

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

MID_XIVELY_C_API_ASMSRCDIR	=
MID_XIVELY_C_API_CSRCDIR	= $(MID_XIVELY_C_API_DIR)/src/libxively \
					$(MID_XIVELY_C_API_DIR)/src/libxively/io/$(XI_IO_LAYER)
MID_XIVELY_C_API_INCDIR		= $(MID_XIVELY_C_API_DIR)/src/libxively \
					$(MID_XIVELY_C_API_DIR)/src/libxively/io/$(XI_IO_LAYER)

ifeq ($(XI_NOB_ENABLED),true)
MID_XIVELY_C_API_CSRCDIR	+= $(MID_XIVELY_C_API_DIR)/src/libxively/nob
MID_XIVELY_C_API_INCDIR		+= $(MID_XIVELY_C_API_DIR)/src/libxively/nob
endif

# find all the source files in the target directories
MID_XIVELY_C_API_CSRCS = $(call get_csrcs, $(MID_XIVELY_C_API_CSRCDIR))
MID_XIVELY_C_API_ASMSRCS = $(call get_asmsrcs, $(MID_XIVELY_C_API_ASMSRCDIR))

# get object files
MID_XIVELY_C_API_COBJS = $(call get_relobjs, $(MID_XIVELY_C_API_CSRCS))
MID_XIVELY_C_API_ASMOBJS = $(call get_relobjs, $(MID_XIVELY_C_API_ASMSRCS))
MID_XIVELY_C_API_OBJS = $(MID_XIVELY_C_API_COBJS) $(MID_XIVELY_C_API_ASMOBJS)

# get dependency files
MID_XIVELY_C_API_DEPS = $(call get_deps, $(MID_XIVELY_C_API_OBJS))

# extra macros to be defined
MID_XIVELY_C_API_DEFINES = -DMID_XIVELY_C_API $(XI_DEFINES)

# genearte library
MID_LIB_XIVELY_C_API = $(OUT_DIR)/libmidxively.a

# library generation rule
$(MID_LIB_XIVELY_C_API): $(MID_XIVELY_C_API_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(MID_XIVELY_C_API_OBJS)

# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions
MID_INCDIR += $(MID_XIVELY_C_API_INCDIR)
MID_CSRCDIR += $(MID_XIVELY_C_API_CSRCDIR)
MID_ASMSRCDIR += $(MID_XIVELY_C_API_ASMSRCDIR)

MID_DEFINES += $(MID_XIVELY_C_API_DEFINES)
MID_DEPS += $(MID_XIVELY_C_API_DEPS)
MID_LIBS += $(MID_LIB_XIVELY_C_API)


# MID_INCDIR += $(MID_XIVELY_C_API_INCDIR)
# MID_CSRCDIR += $(MID_XIVELY_C_API_CSRCDIR)
# MID_ASMSRCDIR += $(MID_XIVELY_C_API_ASMSRCDIR)

# # the dir to generate objs
# MID_XIVELY_C_API_OBJDIR = $(OUT_DIR)/middleware/xively

# # find all the srcs in the target dirs
# MID_XIVELY_C_API_CSRCS = $(call get_csrcs, $(MID_XIVELY_C_API_CSRCDIR))
# MID_XIVELY_C_API_ASMSRCS = $(call get_asmsrcs, $(MID_XIVELY_C_API_ASMSRCDIR))

# # get obj files
# MID_XIVELY_C_API_COBJS = $(call get_objs, $(MID_XIVELY_C_API_CSRCS), $(MID_XIVELY_C_API_OBJDIR))
# MID_XIVELY_C_API_ASMOBJS = $(call get_objs, $(MID_XIVELY_C_API_ASMSRCS), $(MID_XIVELY_C_API_OBJDIR))

# # include dependency files
# ifneq ($(MAKECMDGOALS),clean)
#  -include $(MID_XIVELY_C_API_COBJS:.o=.o.d) $(MID_XIVELY_C_API_ASMOBJS:.o=.o.d)
# endif

# # genearte library
# MID_LIB_XIVELY = $(OUT_DIR)/libmidxively.a
# MID_LIBS += $(MID_LIB_XIVELY)

# $(MID_LIB_XIVELY): $(MID_XIVELY_C_API_OBJDIR) $(MID_XIVELY_C_API_COBJS) $(MID_XIVELY_C_API_ASMOBJS)
# 	$(TRACE_ARCHIVE)
# 	$(Q)$(AR) $(AR_OPT) $@ $(MID_XIVELY_C_API_COBJS) $(MID_XIVELY_C_API_ASMOBJS)

# # specific compile rules
# .SECONDEXPANSION:
# $(MID_XIVELY_C_API_COBJS): $(MID_XIVELY_C_API_OBJDIR)/%.o :$(call get_c_prerequisite, $(MID_XIVELY_C_API_CSRCDIR)) $$(COMMON_COMPILE_PREREQUISITES)
# 	$(TRACE_COMPILE)
# 	$(Q)$(CC) -c $(COMPILE_OPT) $(MID_XIVELY_C_API_COMPILE_OPT) $< -o $@

# .SECONDEXPANSION:
# $(MID_XIVELY_C_API_ASMOBJS): $(MID_XIVELY_C_API_OBJDIR)/%.o :$(call get_asm_prerequisite, $(MID_XIVELY_C_API_ASMSRCDIR)) $$(COMMON_COMPILE_PREREQUISITES)
# 	$(TRACE_ASSEMBLE)
# 	$(Q)$(CC) -c $(ASM_OPT) $(MID_XIVELY_C_API_COMPILE_OPT) $< -o $@

# # generate obj dir
# $(MID_XIVELY_C_API_OBJDIR):
# 	$(TRACE_CREATE_DIR)
# 	$(Q)$(MKD) $(subst /,$(PS),$@)

# # Middleware Definitions
# ifndef XI_USER_CONFIG

# ifeq ($(XI_IO_LAYER),posix)
#   XI_DEFINES += -DXI_IO_LAYER=0
# endif

# ifeq ($(XI_IO_LAYER),contiki)
#   XI_DEFINES += -DXI_IO_LAYER=4
# endif

#   XI_DEFINES += $(foreach constant,$(XI_CONFIG),-D$(constant))
#   XI_DEFINES += -DXI_USER_AGENT=$(XI_USER_AGENT)
#   XI_DEFINES += -DXI_IO_LAYER_POSIX_COMPAT=$(XI_IO_LAYER_POSIX_COMPAT)
# ifeq ($(XI_ALLOCATOR_USE), lwip)
#   XI_DEFINES += -DXI_ALLOCATOR=2
# endif
# else
#   $(warning "Using XI_USER_CONFIG instead of: '$(XI_CONFIG)'")
#   XI_DEFINES += -DXI_USER_CONFIG=1
#   ifdef XI_USER_CONFIG_PATH
#     $(info "Will include '$(XI_USER_CONFIG_PATH)/xi_user_config.h'")
#     XI_DEFINES += -I$(XI_USER_CONFIG_PATH)
#   endif
# endif

# MID_XIVELY_C_API_DEFINES = -DMID_XIVELY_C_API $(XI_DEFINES)
# MID_DEFINES += $(MID_XIVELY_C_API_DEFINES)
LIBXIVELY := $(shell git rev-parse --show-toplevel)

include $(LIBXIVELY)/Makefile.include
include Makefile.defaults

XI_TEST_SUITE ?= core

XI_TEST_SOURCES := $(XI_TEST_SUITE).c $(XI) $(XI_OBJDIR)/tinytest/tinytest.o $(XI_OBJDIR)/xi_event_dispatcher.o $(XI_OBJDIR)/xi_heap.o $(XI_OBJDIR)/xi_static_vector.o

TINYTEST_SRCDIR ?= ../import/tinytest/

XI_CFLAGS += -I./ -I../libxively/ -I$(TINYTEST_SRCDIR) -I../libxively/xi_datastructures -I../libxively/xi_event_dispatcher

ifneq ($(XI_UNIT_TEST_TARGET),native)
  XI_CFLAGS += -DNO_FORKING
endif

XI_TEST = $(XI_BINDIR)/libxively_$(XI_UNIT_TEST_TARGET)_unit_test

.PHONY: $(XI_TEST)
all: $(XI_TEST)

include Makefile.$(XI_UNIT_TEST_TARGET)

$(XI_OBJDIR)/tinytest/tinytest.o: $(TINYTEST_SRCDIR)/tinytest.c
	@-mkdir -p $(dir $@)
	$(CC) $(XI_CFLAGS) -c $< -o $@

$(XI_OBJDIR)/xi_event_dispatcher.o: $(LIBXIVELY)/src/libxively/xi_event_dispatcher/xi_event_dispatcher.c
	@-mkdir -p $(dir $@)
	$(CC) $(XI_CFLAGS) -c $< -o $@

$(XI_OBJDIR)/xi_heap.o: $(LIBXIVELY)/src/libxively/xi_datastructures/xi_heap.c
	@-mkdir -p $(dir $@)
	$(CC) $(XI_CFLAGS) -c $< -o $@

$(XI_OBJDIR)/xi_static_vector.o: $(LIBXIVELY)/src/libxively/xi_datastructures/xi_static_vector.c
	@-mkdir -p $(dir $@)
	$(CC) $(XI_CFLAGS) -c $< -o $@

$(XI):
	$(MAKE) -C .. libxively

include $(LIBXIVELY)/Makefile.rules

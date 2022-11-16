# extra recipes

DEFINES  = -D_POSIX_C_SOURCE=201112L
DEFINES += -DFUSE_USE_VERSION=26

PCFG_CFLAGS := $(shell pkg-config fuse --cflags)
PCFG_LIBS   := $(shell pkg-config fuse --libs)

CPPFLAGS += $(PCFG_CFLAGS)
LDLIBS   += $(PCFG_LIBS)

## compile 'main' source for each program with -fwhole-program
#
#MAINS := $(shell grep -lw '^int main' $(ALL_SRCS))
#
#$(MAINS:$(SPAT)=$(OPAT)): CXXFLAGS += -fwhole-program
#$(MAINS:$(SPAT)=$(DPAT)): CXXFLAGS += -fwhole-program


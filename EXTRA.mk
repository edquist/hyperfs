# extra recipes

DEFINES  = -D_POSIX_C_SOURCE=201112L
DEFINES += -D_FILE_OFFSET_BITS=64

## compile 'main' source for each program with -fwhole-program
#
#MAINS := $(shell grep -lw '^int main' $(ALL_SRCS))
#
#$(MAINS:$(SPAT)=$(OPAT)): CXXFLAGS += -fwhole-program
#$(MAINS:$(SPAT)=$(DPAT)): CXXFLAGS += -fwhole-program


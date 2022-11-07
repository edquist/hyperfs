# extra recipes


## compile 'main' source for each program with -fwhole-program
#
#MAINS := $(shell grep -lw '^int main' $(ALL_SRCS))
#
#$(MAINS:$(SPAT)=$(OPAT)): CXXFLAGS += -fwhole-program
#$(MAINS:$(SPAT)=$(DPAT)): CXXFLAGS += -fwhole-program


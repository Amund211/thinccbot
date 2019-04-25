CXX ?= g++
RM ?= rm -f

CXXFLAGS ?= -Wall -Wextra -Wpedantic

# Install in current folder
prefix ?= .
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin

srcdir ?= ./src
objdir ?= ./obj

.DEFAULT_GOAL := ball

COMPILE = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^
LINK = $(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: all
all: ball

$(bindir):
	mkdir $(bindir)

################# BALL ################
$(objdir)/ball/ball:
	mkdir -p $(objdir)/ball/ball

$(objdir)/ball/%.o: $(srcdir)/%.cpp | $(objdir)/ball/ball ballstates
	$(COMPILE)

.PHONY: ballstates
ballstates:
ifneq ($(shell readlink $(srcdir)/states.h),ball/states.h)
	cd $(srcdir); $(RM) states.h; ln -s ball/states.h states.h
endif

ballsrc = $(shell find $(srcdir) $(srcdir)/ball -maxdepth 1 -type f -name "*.cpp")
ballobj := $(ballsrc:$(srcdir)/%.cpp=$(objdir)/ball/%.o)

.PHONY: ball
ball: $(bindir)/ball
$(bindir)/ball: $(ballobj) | $(bindir)
	$(LINK)

################# UTILITES ################
.PHONY: clean
clean:
	$(RM) -r $(objdir)

.PHONY: distclean
distclean: clean
	$(RM) -r $(bindir)


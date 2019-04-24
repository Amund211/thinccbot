# http://nuclear.mutantstargoat.com/articles/make/#automatic-include-dependency-tracking
#-include $(dep)
# include all dep files in the makefile

# rule to generate a dep file by using the C preprocessor
# (see man cpp for details on the -MM and -MT options)
# %.d: %.c
	# @$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

CXX ?= g++
RM ?= rm -f

# Install in current folder
prefix ?= .
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin

srcdir ?= ./src
objdir ?= ./obj

.DEFAULT_GOAL := ball

basesrc := $(wildcard $(srcdir)/*.cpp)
baseobj := $(basesrc:$(srcdir)/%.cpp=$(objdir)/%.o)

$(objdir)/%.o: $(srcdir)/%.cpp | $(objdir) $(objdir)/ball $(.DEFAULT_GOAL)states
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

.PHONY: all
all:
	$(MAKE) ball

################# BALL ################
$(objdir)/ball:
	mkdir $(objdir)/ball

.PHONY: ballstates
ballstates: $(srcdir)/states.h
	cd $(srcdir); $(RM) states.h; ln -s ball/states.h states.h

ballsrc = $(wildcard $(srcdir)/ball/*.cpp)
ballobj := $(ballsrc:$(srcdir)/%.cpp=$(objdir)/%.o)

.PHONY: ball
ball: $(bindir)/ball
$(bindir)/ball: $(baseobj) $(ballobj) | $(bindir)
	$(CXX) -o $@ $^ $(LDFLAGS)


$(objdir):
	mkdir $(objdir)

$(bindir):
	mkdir $(bindir)

################# UTILITES ################
.PHONY: clean
clean:
	$(RM) -r $(objdir)

.PHONY: distclean
distclean: clean
	$(RM) -r $(bindir)


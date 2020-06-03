CXX ?= g++
RM ?= rm -f

CXXFLAGS ?= -Wall -Wextra -Wpedantic

# Install in current folder
prefix ?= .
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin

srcdir ?= src
objdir ?= obj

.DEFAULT_GOAL := ball

.PHONY: all
all: ball tictactoe connectfour

COMPILE = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^
LINK = $(CXX) -o $@ $^ $(LDFLAGS)

# Do not remote intermediary files
.SECONDARY:


################# GAMES ################
# Shortcuts to avoid having to type `make bin/GAME`
.PHONY: ball
ball: $(bindir)/ball

.PHONY: tictactoe
tictactoe: $(bindir)/tictactoe

.PHONY: connectfour
connectfour: $(bindir)/connectfour


################# RULES ################
$(bindir)/:
	mkdir -p $@

# Make objdir and any subdirectories
$(objdir)/:
	mkdir -p $@

$(objdir)/%/:
	mkdir -p $@

.SECONDEXPANSION:
$(objdir)/%.o: $(srcdir)/%.cpp | $$(dir $$(objdir)/%)
	$(COMPILE)

PERCENT := %
$(bindir)/%: $$(patsubst $$(srcdir)/$$(PERCENT).cpp,$$(objdir)/$$(PERCENT).o,$$(wildcard $$(srcdir)/*.cpp) $$(wildcard $$(srcdir)/%/*.cpp)) | $(bindir)/

	$(LINK)


################# UTILITES ################
.PHONY: clean
clean:
	$(RM) -r $(objdir)

.PHONY: distclean
distclean: clean
	$(RM) -r $(bindir)

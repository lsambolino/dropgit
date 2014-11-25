
# This Makefile behaves as follows:
# - it builds compilation dependencies based on compiler's features (work
#   with GNU gcc/g++, to be checked for other compilers).
# - it compiles sources in the SRCDIR folder (up to one nested levels);
# - it puts object files in the OBJDIR folders (no subfolders);
# - it builds the executable file, puts it into the EXEDIR folder with name EXENAME.
# 
# Available targets:
# - test_names:    prints out a list of internal variable for debugging purpose)
# - depend:			 builds dependency for the project
# - clean:			 purges executable and all objects
# - distclean:		 purges executable, all objects and all dependencies
# - all:				 builds all the project
#

DIR=$(strip $(shell pwd))
SRCDIR= src
OBJDIR= objects
DEPDIR= dependencies
EXEDIR= bin
EXENAME= test

SUFFIXES += .d

CXX= g++

CXXFLAGS= -std=c++0x -frtti -g -Wall -Wextra -Winline -Wno-unknown-pragmas \
    -pedantic -Wmain -Wswitch-default -Wswitch-enum -Wmissing-include-dirs \
    -Wmissing-declarations -Wundef -Wcast-align -Wredundant-decls -Winit-self \
    -Woverloaded-virtual -DHAVE_CXA_DEMANGLE

# This flags work well with g++. Must be checked
# for other compilers.
CXXDEPFLAGS= -MM

LDFLAGS= -lboost_serialization -lboost_program_options -lboost_system -lboost_filesystem -lpthread -ldl
	 
SOURCES= $(shell find $(SRCDIR) -name "*.cpp")

VPATH=$(sort $(dir $(SOURCES)))

OBJS=$(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))
DEPFILES=$(addprefix $(DEPDIR)/, $(notdir $(SOURCES:.cpp=.d)))

EXECUTABLE=	$(EXEDIR)/$(EXENAME)

ifneq ($(CXX),g++)
	@echo "============================"
	@echo "Please check this compiler  "
	@echo "support -MM flag to build   "
	@echo "dependencies.               "
	@echo "============================"
endif

all: $(EXECUTABLE)

$(EXECUTABLE):	$(OBJS) Makefile
	@echo "* Building " $@"..."
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "*** Done! ***"
	

$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	@echo "* Compiling " $<"..."
	@$(CXX) $(CXXFLAGS) -c $< -o $(OBJDIR)/$(notdir $@)

$(DEPDIR)/%.d: %.cpp
	@echo "* Building dependencies for " $<"..."
	@$(CXX) $(CXXFLAGS) $(CXXDEPEND) $(CXXDEPFLAGS) $< \
		| sed -e "s/^.*\.o/$(OBJDIR)\/& $(subst /,\/,$@) /" > $@ 

test_names:
	@echo "Dir:             " "**"$(DIR)"**"
	@echo "Source files:    " $(SOURCES)
	@echo "Source folders:  " $(sort $(dir $(SOURCES)))
	@echo "Object files:    " $(OBJS)
	@echo "Object dir:      " $(OBJDIR)
	@echo "Executable:      " $(EXECUTABLE)
	@echo "Dep files:       " $(DEPFILES)
	@echo "Suffixes:        " $(SUFFIXES)

clean:
	rm -f $(OBJS) $(EXECUTABLE)

distclean: clean
	rm -f $(DEPFILES)

depend: $(DEPFILES)
	@echo "*** Done! ***"

.PHONY: test_names clean all depend distclean

-include $(DEPFILES)
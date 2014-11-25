
# This Makefile behaves as follows:
# - it builds compilation dependencies based on compiler's features (work
#   with GNU gcc/g++, to be checked for other compilers).
# - it compiles sources in the SOURCES;
# - it puts object files in the OBJDIR folders (no subfolders);
# - it builds the executable file, puts it into the EXEDIR folder with name EXENAME.
# 
# Available targets:
# - test_names:     prints out a list of internal variable for debugging purpose)
# - depend:		    builds dependency for the project
# - clean:			purges executable and all objects
# - distclean:		purges executable, all objects and all dependencies
# - all:		    builds all the project
#

################################################################################################
#                                       CONFIGURATION

EXENAME:= gal_rest

CXX:= clang++

CXXFLAGS:= -std=c++1y -frtti -g -Wall -O3 -Wextra -Wno-unknown-pragmas \
    -pedantic -Wmain -Wswitch-default -Wswitch-enum -Wmissing-include-dirs \
    -Wmissing-declarations -Wundef -Wcast-align -Wredundant-decls -Winit-self \
    -Woverloaded-virtual -Winline -Wdeprecated -Wvexing-parse -Werror

INCLUDES:= -I../../framework -I../../framework/common -I../../lib -I.

LDFLAGS:= -lboost_filesystem -lboost_system -lpthread

################################################################################################

DIR=$(strip $(shell pwd))

OBJDIR:= objs
DEPDIR:= deps
EXEDIR:= bin

SUFFIXES += .d

# This flags work well with g++. Must be checked
# for other compilers.
CXXDEPFLAGS:= -MM

SOURCES=	$(DIR)/main.cpp \
			$(DIR)/real_device.cpp \
            $(DIR)/../../lib/activity/http/gal_controller.cpp \
            $(DIR)/../../lib/gal_drop/gsi.cpp \
            $(DIR)/../../framework/activity/http/controller.cpp \
            $(DIR)/../../framework/activity/http/web_controller.cpp \
            $(DIR)/../../framework/activity/http/http_connection.cpp \
			$(DIR)/../../framework/activity/http/http_server.cpp \
			$(DIR)/../../framework/activity/activity_from_event.cpp \
            $(DIR)/../../framework/activity/concurrent_activity.cpp \
            $(DIR)/../../framework/common/application.cpp \
			$(DIR)/../../framework/common/active_object.cpp  \
			$(DIR)/../../framework/common/colors.cpp \
			$(DIR)/../../framework/common/demangle.cpp \
			$(DIR)/../../framework/common/ip_address.cpp \
			$(DIR)/../../framework/common/ip_socket_address.cpp \
			$(DIR)/../../framework/common/init_sockets.cpp \
			$(DIR)/../../framework/common/log.cpp \
            $(DIR)/../../framework/util/system.cpp \
            $(DIR)/../../framework/util/pugixml.cpp \
            $(DIR)/../../framework/util/string.cpp \
			$(DIR)/../../framework/event/event_from_message.cpp \
			$(DIR)/../../framework/exception/exception.cpp \
			$(DIR)/../../framework/io/tcp_io.cpp \
            $(DIR)/../../framework/protocol/async_protocol.cpp \
			$(DIR)/../../framework/protocol/http/http_headers.cpp \
			$(DIR)/../../framework/protocol/http/http_method.cpp \
			$(DIR)/../../framework/protocol/http/http_protocol.cpp \
			$(DIR)/../../framework/protocol/http/http_response.cpp \
			$(DIR)/../../framework/protocol/http/http_status_code.cpp \
			$(DIR)/../../framework/protocol/http/cpprest/uri.cpp \
			$(DIR)/../../framework/protocol/http/cpprest/uri_builder.cpp \
			$(DIR)/../../framework/protocol/http/cpprest/uri_parser.cpp

VPATH:= $(sort $(dir $(SOURCES)))
OBJS:= $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))
DEPFILES:= $(addprefix $(DEPDIR)/, $(notdir $(SOURCES:.cpp=.d)))

EXECUTABLE:= $(EXEDIR)/$(EXENAME)

all: $(EXECUTABLE)

$(EXECUTABLE):	$(OBJS) Makefile
	@echo "* Building " $@"..."
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	@echo "*** Done! ***"

$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	@echo "* Compiling " $<"..."
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $(OBJDIR)/$(notdir $@)

$(DEPDIR)/%.d: %.cpp
	@echo "* Building dependencies for " $<"..."
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(CXXDEPEND) $(CXXDEPFLAGS) $< | sed -e "s/^.*\.o/$(OBJDIR)\/& $(subst /,\/,$@) /" > $@ 

test_names:
	@echo "Dir:             " $(DIR)
	@echo "ExcludeDir:      " $(EXCLUDES)
	@echo "Source files:    " $(SOURCES)
	@echo "Source folders:  " $(sort $(dir $(SOURCES)))
	@echo "Object files:    " $(OBJS)
	@echo "Object dir:      " $(OBJDIR)
	@echo "Executable:      " $(EXECUTABLE)
	@echo "Dep files:       " $(DEPFILES)
	@echo "Suffixes:        " $(SUFFIXES)

clean:
	rm -f $(OBJS) $(EXECUTABLE) $(DEPFILES)
	for dir in $(LIBDIRS); do (cd $$dir; ${MAKE} clean); done

distclean: clean
	rm -f $(DEPFILES)
	for dir in $(LIBDIRS); do (cd $$dir; ${MAKE} distclean); done

depend: $(DEPFILES)
	@echo "*** Done! ***"

.PHONY: test_names clean all depend distclean

-include $(DEPFILES)

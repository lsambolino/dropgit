
# Predefined variables:
#
# ${TARGET}: the target passed to the build++ application.
# ${HOST}: the host name.

# Needed variables:
#
# CXX: the invoked compiler.
# BUILDDIR: the directory where to place the object files.
# PROGRAM: the executable name.
# SRCDIR: one or more directories where the source files are.

# Optional variables:
#
# CXXFLAGS: the flags passed to the compiler.
# LDFLAGS: the flags passed to the linker.
# INCLUDES: the directories where to search for the include files (the -I flags).

CXX= clang++

CXXFLAGS= -std=c++1y -frtti -g -Wall -O3 -Wextra -Wno-unknown-pragmas \
    -pedantic -Wmain -Wswitch-default -Wswitch-enum -Wmissing-include-dirs \
    -Wmissing-declarations -Wundef -Wcast-align -Wredundant-decls -Winit-self \
    -Woverloaded-virtual -DWITH_BOOST -Winline -Wvexing-parse -Werror
#-Wdeprecated

LDFLAGS= -lboost_serialization -lboost_program_options -lboost_system -lboost_filesystem \
	-lpthread -ldl -rdynamic -latomic

INCLUDES= -Isrc/framework -Isrc/framework/common -Isrc/lib -Isrc/${TARGET}

BUILDDIR= ../build/${HOST}/${TARGET}

PROGRAM= src/${TARGET}/${TARGET}

SRCDIR= src/${TARGET}
SRCDIR= src/lib
SRCDIR= src/framework

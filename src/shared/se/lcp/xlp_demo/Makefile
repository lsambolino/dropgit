
ROOTDIR= $(realpath ../../../..)

CXX=g++
#CXX=clang++

CXXFLAGS=   -std=c++1y -O3 -frtti -g -Wall -Wextra -Wno-unknown-pragmas \
            -pedantic -Wmain -Wswitch-default -Wswitch-enum -Wmissing-include-dirs \
            -Wmissing-declarations -Wundef -Wcast-align -Wredundant-decls -Winit-self \
            -Woverloaded-virtual -fPIC -I$(ROOTDIR)/lib -I$(ROOTDIR)/framework -I$(ROOTDIR)/framework/common -I$(ROOTDIR)/fe

LIBNAME=	XLPDemoLCP.so

all:
	$(CXX) $(CXXFLAGS) instance.cpp demo_lcp.cpp -shared -Wl,-soname,$(LIBNAME).1 -o $(LIBNAME)

install:
	cp $(LIBNAME) $(ROOTDIR)/fe

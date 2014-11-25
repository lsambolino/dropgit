
CXX=clang++

CXXFLAGS=   -std=c++1y -O3 -frtti -g -Wall -Wextra -Winline -Wno-unknown-pragmas \
            -pedantic -Wmain -Wswitch-default -Wswitch-enum -Wmissing-include-dirs \
            -Wmissing-declarations -Wundef -Wcast-align -Wredundant-decls -Winit-self \
            -Woverloaded-virtual -Werror -fPIC

PROGNAME=	bridge

all:
	$(CXX) $(CXXFLAGS) main.cpp forward.cpp file_descriptor.cpp log.cpp -o $(PROGNAME) -lpthread -latomic -lboost_program_options

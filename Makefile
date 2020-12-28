inc := -I/Library/Frameworks/SDL2.framework/Headers -I include -I/Users/nathanklundt/Downloads/boost_1_75_0
fw := -framework SDL2 -F/Library/Frameworks
lib := -L/Users/nathanklundt/Downloads/boost_1_75_0/stage/lib
cc := g++ -std=c++17 -Wall -O3

main: main.o
	$(cc) $(fw) -o main main.o $(lib)

main.o: main.cpp
	$(cc) -c $< $(inc)

vpath %.cpp src
vpath %.hpp include


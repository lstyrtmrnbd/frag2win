CC = g++
CFLAGS = -Wall -Wextra -ggdb

LDFLAGS = -lopengl32 -lglew32 -lglfw3 -lpthread -lSOIL -static-libgcc -static-libstdc++

TARGET = frag2win

SOURCES = $(wildcard source/*.cpp)
OBJ = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) 


# Compiler and Toolchain
CXX = x86_64-w64-mingw32-g++
LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -lpthread -lws2_32
OPTFLAGS = -O2

# Paths
RAYLIB_PATH = ./raylib-5.5_win64_mingw-w64
INCLUDES = -I$(RAYLIB_PATH)/include
LDFLAGS = -L$(RAYLIB_PATH)/lib

# Project Files
SOURCES = main.cpp drawing.cpp phys.cpp
OBJ = $(SOURCES:.cpp=.o)
TARGET = pool_game.exe

# Build Rule
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(SOURCES) -o $(TARGET) $(OPTFLAGS) $(INCLUDES) $(LDFLAGS) $(LDLIBS) -static

clean:
	rm -f $(TARGET)

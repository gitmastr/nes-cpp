# https://stackoverflow.com/questions/30573481/path-include-and-src-directory-makefile

EXE = nescpp
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
SRC = $(wildcard $(SRC_DIR)/*.cpp)
HDR = $(wildcard $(INCLUDE_DIR)/*.hpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CPPFLAGS += -std=c++17 -Wall -Wpedantic -I$(INCLUDE_DIR) -g3 -Og
# CPPFLAGS += -std=c++17 -Wall -I$(INCLUDE_DIR) -O3 -flto
LDFLAGS += -Llib
LDLIBS += -lm -lSDL2main -lSDL2
LDLIBSWIN += -lm -lmingw32 -lSDL2main -lSDL2
CC = g++

all: $(EXE)

run: $(EXE)
	./$(EXE)

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXE)

time:
	time ./$(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

windows: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBSWIN) -o $(EXE).exe

windows_run: windows
	./$(EXE).exe roms/dk.nes

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDR)
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(EXE)

.PHONY: all clean

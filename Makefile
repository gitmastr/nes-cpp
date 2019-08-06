# https://stackoverflow.com/questions/30573481/path-include-and-src-directory-makefile

EXE = nescpp
SRC_DIR = src
OBJ_DIR = obj
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CPPFLAGS += -std=c++17 -Wall -Iinclude -g -D_GLIBCXX_DEBUG
# CPPFLAGS += -std=c++17 -Wall -Iinclude -O3
LDFLAGS += -Llib
LDLIBS += -lm -lSDL2
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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(EXE)

.PHONY: all clean

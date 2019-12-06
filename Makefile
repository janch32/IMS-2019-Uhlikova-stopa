EXE=sim

SRC_DIR := src
OBJ_DIR := obj
MODE := release

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CFLAGS += -std=c++11 -pedantic -Wall -Wextra -g
LDFLAGS += -std=c++11
LDLIBS += -lstdc++

.PHONY: all debug clean test

all: $(EXE)

debug: CFLAGS += -DDEBUG -g
debug: all

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: clean debug
	@chmod +x ./test/test.js
	./test/test.js
	@$(RM) $(OBJ)
	@$(RM) $(EXE)

clean:
	$(RM) $(OBJ)
	$(RM) $(EXE)
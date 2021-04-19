CXX = g++
CXXFLAGS = -Wall -std=c++11 -g -o

SRC_DIR = ./src
BIN_DIR = ./bin

TARGET_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
TARGET_TEMP = $(foreach target_src, $(TARGET_SRCS), $(subst $(SRC_DIR), $(BIN_DIR), $(target_src)))
TARGET = $(TARGET_TEMP:.cpp=.out)

all: $(TARGET)

$(TARGET): $(BIN_DIR)/%.out: $(SRC_DIR)/%.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $@ $<

clean:
	rm -rf $(BIN_DIR)




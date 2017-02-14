BUILD=Debug
TARGET=$(BUILD)/VulkanSandbox

SHADER_SRC=$(wildcard Shaders/*) $(wildcard Shaders/*/*)
SHADER_OBJ=$(patsubst %, $(BUILD)/%.spv, $(SHADER_SRC))

C_SRC=$(wildcard Source/*.c) $(wildcard Source/*/*.c)
SRC=$(wildcard Source/*.cpp) $(wildcard Source/*/*.cpp)
HEADERS=$(wildcard Source/*.h) $(wildcard Source/*/*.h)
OBJ=$(patsubst %.cpp, $(BUILD)/%.o, $(SRC)) $(patsubst %.c, $(BUILD)/%.o, $(C_SRC))

INCLUDES=-I../Engine/Include -I../Middleware/Include
CFLAGS=-g $(INCLUDES)
CPPFLAGS=-std=c++11 -g $(INCLUDES)
LDFLAGS=-L../Engine/Debug -lglfw -lvulkan -lEngine

all: $(SHADER_OBJ) $(TARGET)

$(BUILD)/%.spv: %
	mkdir -p $(dir $@) && glslangValidator -V $< -o $@

$(TARGET): $(OBJ)
	g++ $(OBJ) $(LDFLAGS) -o $@

$(BUILD)/%.o: %.cpp $(HEADERS)
	mkdir -p $(dir $@) && g++ $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
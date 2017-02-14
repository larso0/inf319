BUILD=Debug
TARGET=$(BUILD)/VulkanSandbox

SHADER_SRC=$(wildcard Shaders/*) $(wildcard Shaders/*/*)
SHADER_OBJ=$(patsubst %, $(BUILD)/%.spv, $(SHADER_SRC))

SRC=$(wildcard Source/*.cpp) $(wildcard Source/*/*.cpp)
HEADERS=$(wildcard Source/*.h) $(wildcard Source/*/*.h)
OBJ=$(patsubst %.cpp, $(BUILD)/%.o, $(SRC))

CFLAGS=-std=c++11
LDFLAGS=-lglfw -lvulkan

all: $(SHADER_OBJ) $(TARGET)

$(BUILD)/%.spv: %
	mkdir -p $(dir $@) && glslangValidator -V $< -o $@

$(TARGET): $(OBJ)
	g++ $(OBJ) $(LDFLAGS) -o $@

$(BUILD)/%.o: %.cpp $(HEADERS)
	mkdir -p $(dir $@) && g++ $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
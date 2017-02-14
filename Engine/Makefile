BUILD=Debug
TARGET=$(BUILD)/libEngine.a

C_SRC=$(wildcard Source/*.c) $(wildcard Source/*/*.c)
SRC=$(wildcard Source/*.cpp) $(wildcard Source/*/*.cpp)
HEADERS=$(wildcard Source/*.h) $(wildcard Source/*/*.h)
OBJ=$(patsubst %.cpp, $(BUILD)/%.o, $(SRC)) $(patsubst %.c, $(BUILD)/%.o, $(C_SRC))

INCLUDES=-IInclude -I../Middleware/Include
CFLAGS=-g $(INCLUDES)
CPPFLAGS=-std=c++11 -g $(INCLUDES)

all: $(TARGET)

$(TARGET): $(OBJ)
	ar -r $@ $(OBJ)

$(BUILD)/%.o: %.cpp $(HEADERS)
	mkdir -p $(dir $@) && g++ $(CPPFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c $(HEADERS)
	mkdir -p $(dir $@) && gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)


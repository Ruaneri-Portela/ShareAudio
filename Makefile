CC := gcc
CFLAGS := -Wall -Wextra -pedantic
LIBS := -lportaudio
ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32 
else
    LIBS += -lpthread -lrt
endif

SRCS := $(wildcard src/*.c)
BUILD_DIR := ./build
OBJS := $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))
TARGET := $(BUILD_DIR)/ShareAudio

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o : src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	@echo Build complete for ShareAudio

clean:
	rm -r $(BUILD_DIR)/*
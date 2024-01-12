CC := gcc
CFLAGS := -Wall
LIBS :=
ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32 -lportaudio
endif

SRCS := $(wildcard *.c)
BUILD_DIR := ./build
OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
TARGET := $(BUILD_DIR)/sharedaudio

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -r $(BUILD_DIR)/*

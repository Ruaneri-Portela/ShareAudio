CC := gcc
CFLAGS := -Wall -Wextra -pedantic
LIBS := -lportaudio
WINRES = ""
WINRES_O = ""
BUILD_DIR := build/
SRC_DIR = src/

ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32
    WINRES_O = $(BUILD_DIR)res.o
    WINRES = windres $(SRC_DIR)Resource.rc -O coff -o $(WINRES_O)
else
    LIBS += -lpthread
endif

SRCS := $(wildcard $(SRC_DIR)*.c)
OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.c=.o)))
TARGET := $(BUILD_DIR)ShareAudio

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)%.o : $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(WINRES)
	$(CC) $(CFLAGS) $^ $(WINRES_O) -o $@ $(LIBS)
	@echo Build complete for ShareAudio

clean:
	rm -r $(BUILD_DIR)*

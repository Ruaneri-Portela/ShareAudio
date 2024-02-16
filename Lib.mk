CC := gcc
CFLAGS := -Wall -Wextra -pedantic
BUILD_DIR := build/
SRC_DIR = src/
LIBS := -lportaudio -lcrypto
DLLEXPORT =
NAME := libShareAudio
PORTAUDIO := ../portaudio/include/portaudio.h

ifeq ($(wildcard $(PORTAUDIO)),)
    $("O arquivo $(PORTAUDIO) não foi encontrado.")
else
	CFLAGS += -I./portaudio/include/
	LIBS += -lm
endif

ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32
	TARGET := $(BUILD_DIR)$(NAME).dll
	DLLEXPORT = -D'DLL_EXPORT'
else
    LIBS += -lpthread
	TARGET := $(BUILD_DIR)$(NAME).so
endif

SRCS := $(wildcard $(SRC_DIR)*.c)
OBJS := $(filter-out $(BUILD_DIR)ShareAudio.o,$(addprefix $(BUILD_DIR), $(notdir $(SRCS:.c=.o))))

all: $(BUILD_DIR) $(TARGET)
$(BUILD_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -shared -fPIC $(DLLEXPORT) -c $< -o $@
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -fPIC $(DLLEXPORT) $^ -o $@ $(LIBS) -Wl,--out-implib,$(BUILD_DIR)$(NAME).a
	@echo Build complete for libShareAudio

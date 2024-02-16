CC := gcc
CFLAGS := -Wall -Wextra -pedantic
BUILD_DIR := build/
SRC_DIR := src/
LIBS := -L./$(BUILD_DIR)  -lShareAudio
WINRES :=
WINRES_O :=
ifeq ($(OS),Windows_NT)
    WINRES_O = $(BUILD_DIR)res.o
    WINRES = windres $(SRC_DIR)Resource.rc -O coff -o $(WINRES_O)
else
endif

SRCS :=  $(BUILD_DIR)ShareAudio.c
OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.c=.o)))

TARGET := $(BUILD_DIR)ShareAudio

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR)%.o : $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(WINRES)
	$(CC) $(CFLAGS) $^ $(WINRES_O) -o $@ $(LIBS)
	@echo Build complete for ShareAudio
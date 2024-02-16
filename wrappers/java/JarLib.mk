CC := gcc
CFLAGS := -Wall -Wextra -pedantic
BUILD_DIR := ../../build/
SRC_DIR := ./
LIBS := -lShareAudio
NAME := libShareAudioJava

INCLUDEJNI := include

ifeq ($(OS),Windows_NT)
    TARGET := $(BUILD_DIR)$(NAME).dll
else
    TARGET := $(BUILD_DIR)$(NAME).so
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.c=.o)))

SRCSJAVA := $(wildcard $(SRC_DIR)/*.java)
CLASSES := $(addprefix $(SRC_DIR), $(notdir $(SRCSJAVA:.java=.class)))
JAR := $(BUILD_DIR)ShareAudio.jar

all: $(CLASSES) $(JAR) $(TARGET)

$(SRC_DIR)%.class:
	javac -h $(SRC_DIR) $(SRC_DIR)*.java -d $(SRC_DIR)

$(JAR):
	jar cvfe $@ ShareAudio $(SRC_DIR)*.class
	@echo Build complete for ShareAudio.jar

$(BUILD_DIR)%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -shared -fPIC -I./$(INCLUDEJNI) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -shared -fPIC $^ -o $@ -L./$(BUILD_DIR) $(LIBS)
	@echo Build complete for $(NAME)

clean:
	rm -f ./*.class
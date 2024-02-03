CC := gcc
CFLAGS := -Wall -Wextra -pedantic
LIBS := -lportaudio
WINRES =
WINRES_O =
BUILD_DIR := build/
SRC_DIR = src/
DLLEXPORT =
ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32
    WINRES_O = $(BUILD_DIR)res.o
    WINRES = windres $(SRC_DIR)Resource.rc -O coff -o $(WINRES_O)
	LIBTARGET := $(BUILD_DIR)libShareAudio.dll
	DLLEXPORT = -D'DLL_EXPORT'
else
    LIBS += -lpthread
	LIBTARGET := $(BUILD_DIR)libShareAudio.so
endif

SRCS := $(wildcard $(SRC_DIR)*.c)
OBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.c=.o)))
LOBJS := $(addprefix $(BUILD_DIR), $(notdir $(SRCS:.c=_so.o)))
LIBOBJS = $(filter-out $(BUILD_DIR)ShareAudio_so.o, $(LOBJS))

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
	make lib

lib: $(BUILD_DIR) $(LIBTARGET)
$(BUILD_DIR)%_so.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) -shared -fPIC $(DLLEXPORT) -c $< -o $@
$(LIBTARGET): $(LIBOBJS)
	$(CC) $(CFLAGS) -shared -fPIC $(DLLEXPORT) $^ -o $@ $(LIBS)
	@echo Build complete for libShareAudio

clean:
	rm -r $(BUILD_DIR)*

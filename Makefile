BUILD_DIR := build/
all: $(BUILD_DIR)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
all:
	make -f Lib.mk
	make -f Exec.mk

java:
	make
	cd wrappers/java && make -f JarLib.mk
	
clean:
	rm -r $(BUILD_DIR)*
	cd wrappers/java && make -f JarLib.mk clean
	
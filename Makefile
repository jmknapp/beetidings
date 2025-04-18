# Directory containing the sub-makefile
SUBDIR = sens/raspberry-pi-i2c-sen5x

# Default target
all:
	$(MAKE) -C $(SUBDIR)

# Pass-through clean
clean:
	$(MAKE) -C $(SUBDIR) clean

.PHONY: all clean

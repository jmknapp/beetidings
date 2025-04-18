# Compiler
CC = gcc

# Detect MySQL or MariaDB client library
MYSQL_CONFIG := $(shell which mysql_config 2>/dev/null)
MARIADB_CONFIG := $(shell which mariadb_config 2>/dev/null)

ifeq ($(strip $(MYSQL_CONFIG)),)
  ifeq ($(strip $(MARIADB_CONFIG)),)
    DB_CFLAGS = -I/usr/include/mysql
    DB_LIBS = -lmysqlclient
  else
    DB_CFLAGS = $(shell $(MARIADB_CONFIG) --cflags)
    DB_LIBS = $(shell $(MARIADB_CONFIG) --libs)
  endif
else
  DB_CFLAGS = $(shell $(MYSQL_CONFIG) --cflags)
  DB_LIBS = $(shell $(MYSQL_CONFIG) --libs)
endif

# Compiler and linker flags
CFLAGS = -Wall -Wextra -O2 -g $(DB_CFLAGS)
LDFLAGS = $(DB_LIBS)

# Logical source groups
common_sources = sensirion_config.h sensirion_common.h sensirion_common.c
i2c_sources = sensirion_i2c.h sensirion_i2c.c
i2c_implementation = sensirion_i2c_hal.h sensirion_i2c_hal.c
sen5x_sources = sen5x_i2c.h sen5x_i2c.c

# Targets
TARGETS = sen5x_i2c_example_usage senslog airdb

# Source file sets for each target
sen5x_i2c_example_usage_SRCS = sen5x_i2c_example_usage.c \
                                $(filter %.c, $(sen5x_sources) $(i2c_sources) $(i2c_implementation) $(common_sources))

senslog_SRCS = senslog.c \
               $(filter %.c, $(sen5x_sources) $(i2c_sources) $(i2c_implementation) $(common_sources))

airdb_SRCS = airdb.c \
             $(filter %.c, $(sen5x_sources) $(i2c_sources) $(i2c_implementation) $(common_sources))

# Object file mapping
sen5x_i2c_example_usage_OBJS = $(sen5x_i2c_example_usage_SRCS:.c=.o)
senslog_OBJS = $(senslog_SRCS:.c=.o)
airdb_OBJS = $(airdb_SRCS:.c=.o)

# Default build rule
all: $(TARGETS)

# Individual binary rules
sen5x_i2c_example_usage: $(sen5x_i2c_example_usage_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

senslog: $(senslog_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

airdb: $(airdb_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Object file compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cleanup
clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean


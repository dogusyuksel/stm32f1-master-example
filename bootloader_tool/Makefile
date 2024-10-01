CC = gcc
RM = rm -rf
CFLAGS = -Wall -Wextra -g3 -O0 -lpthread -Wno-deprecated-declarations
all_c_files := $(wildcard *.c)

.PHONY: all clean

all:
	$(CC) $(all_c_files) -o "serial.exe" $(CFLAGS); \

partial:
	$(CC) $(FILE) -o $(FILE)".exe" $(CFLAGS); \

clean:
	for file in $(all_c_files); do \
		$(RM) -rf $$file".exe"; \
	done

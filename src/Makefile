cc = gcc
CCFLAGS =

SRCS = main.c

TARGET = cache-sim

OBJS := $(patsubst %.c,%.o,$(SRCS))

all: $(TARGET)

%.o:%.c
			$(CC) $(CCFLAGS) $< -c -o $@ -lm

$(TARGET): $(OBJS)
			$(CC) $(CCFLAGS) $^ -o $@ -lm

.PHONY = clean
.PHONY = run
run: $(TARGET)
			./$(TARGET) $(ARGS)

clean:
			rm -f $(OBJS) $(TARGET)

CC = gcc
LD = gcc

CFLAGS = -O0 -g -Wall -Werror -Wpedantic
LDFLAGS = -lSDL2

BUILD = build
TARGET = $(BUILD)/cnes

SRCS = src/main.c
OBJS = $(SRCS:src/%.c=$(BUILD)/%.o)
DEPS = $(OBJS:.o=.d)


.PHONY: clean run


$(TARGET): $(OBJS)
	mkdir -p $(BUILD)
	$(LD) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BUILD)/%.o: src/%.c
	mkdir -p build
	gcc $(CFLAGS) -MMD -MP $< -c -o $@


clean:
	rm -rf $(BUILD)

run: $(TARGET)
	./$(TARGET)

-include $(DEPS)

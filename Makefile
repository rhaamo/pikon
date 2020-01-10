TARGET ?= pikon
SRC_DIRS ?= ./src

CC = c++


SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s) ./log.c/src/log.c
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d) ./log.c/src/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

FLAGS_LIBSERIALPORT = $(shell pkg-config --cflags libserialport)
LIBS_LIBSERIALPORT = $(shell pkg-config --libs libserialport)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++11 $(FLAGS_LIBSERIALPORT) -DLOG_USE_COLOR -O0

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS) $(LIBS_LIBSERIALPORT)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)

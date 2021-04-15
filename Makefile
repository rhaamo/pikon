TARGET ?= pikon
SRC_DIRS ?= ./src

CC = c++

EXT_SRCS := ./ext/log.c/src/log.c

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s) $(EXT_SRCS)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

EXT_INCS := ./ext/log.c/src/ ./ext/CLI11/include

INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(EXT_INCS)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

FLAGS_LIBSERIALPORT = $(shell pkg-config --cflags libserialport)
LIBS_LIBSERIALPORT = $(shell pkg-config --libs libserialport)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP  $(FLAGS_LIBSERIALPORT) -DLOG_USE_COLOR -O0 -ggdb -fno-inline

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS) $(LIBS_LIBSERIALPORT)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)

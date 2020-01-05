TARGET ?= pikon
SRC_DIRS ?= ./src

CC = c++

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INCDIRS))

FLAGS_LIBSERIALPORT = $(shell pkg-config --cflags libserialport)
LIBS_LIBSERIALPORT = $(shell pkg-config --libs libserialport)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++11 $(FLAGS_LIBSERIALPORT)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LOADLIBES) $(LDLIBS) $(LIBS_LIBSERIALPORT)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)

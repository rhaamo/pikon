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

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP $(FLAGS_LIBSERIALPORT) -DLOG_USE_COLOR -O0 -ggdb -fno-inline

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ -lstdc++ $(LOADLIBES) $(LDLIBS) $(LIBS_LIBSERIALPORT)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

cppcheck:
	cppcheck -Iext/log.c/src -Iext/CLI11/include --std=c++11 src/

cppcheckall:
	cppcheck -Iext/log.c/src -Iext/CLI11/include --enable=all --std=c++11 src/

-include $(DEPS)

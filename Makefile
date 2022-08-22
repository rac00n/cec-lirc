PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = cec-lirc.o
LDFLAGS = -ldl

ifeq ($(BUILD_MODE),debug)
	CFLAGS += -g
else ifeq ($(BUILD_MODE),linuxtools)
	CFLAGS += -g -pg -fprofile-arcs -ftest-coverage
	LDFLAGS += -pg -fprofile-arcs -ftest-coverage
	EXTRA_CLEAN += cec-lirc.gcda cec-lirc.gcno $(PROJECT_ROOT)gmon.out
	EXTRA_CMDS = rm -rf cec-lirc.gcda
else
	CFLAGS += -O2
endif


all:	cec-lirc

cec-lirc:	$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	$(EXTRA_CMDS)

%.o:	$(PROJECT_ROOT)%.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

%.o:	$(PROJECT_ROOT)%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -fr cec-lirc $(OBJS) $(EXTRA_CLEAN)

PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

OBJS = cec-lirc.o
LDFLAGS = -ldl -llirc_client
CFLAGS += -Wall

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

# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install:	cec-lirc
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $< $(DESTDIR)$(PREFIX)/bin/
	install -m 644 -C systemd/cec-lirc.service /etc/systemd/system/cec-lirc.service

clean:
	rm -fr cec-lirc $(OBJS) $(EXTRA_CLEAN)

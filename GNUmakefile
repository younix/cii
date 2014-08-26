OS := $(shell uname -s)

# GNU/Linux
ifeq ($(OS), Linux)
	CFLAGS_BSD := `pkg-config --cflags libbsd`
	CFLAGS_CURSES=`pkg-config --cflags ncurses`
	LIBS_BSD := `pkg-config --libs libbsd`
	LIB_CURSES := `pkg-config --libs ncurses`
	DEFINES := -DUSE_LIBBSD -D_GNU_SOURCE
	WARN=-Wno-unused-but-set-variable
endif

# MacOSX
ifeq ($(OS), Darwin)
	DEFINES := -D_DARWIN_C_SOURCE
endif

include Makefile

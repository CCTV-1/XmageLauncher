CC=g++
FLAGS=-Wall -Wextra -Wpedantic -std=gnu++17 -m64
GLIB_COMPILE_RESOURCES=glib-compile-resources
LIBZIP_FLAGS=$(shell pkg-config --cflags --libs libzip)
CURL_FLAGS=$(shell pkg-config --cflags --libs libcurl)
GTKMM_FLAGS=$(shell pkg-config --cflags --libs gtkmm-3.0)
GLIBMM_FLAGS=$(shell pkg-config --cflags glibmm-2.4)
JANSSON_FLAGS=$(shell pkg-config --cflags --libs jansson)

ifeq ($(CC),g++)
	FLAGS+=-lstdc++fs
else
	FLAGS+=-c++fs
endif

ifndef DEBUG
	FLAGS+=-O3 
endif
ifdef DEBUG
	FLAGS+=-O0 -g3 -pg -DDEBUG
endif

launcher: src/launcher.cpp src/launcher-resources.cpp fileutilities.o networkutilities.o
	$(CC) src/launcher.cpp src/launcher-resources.cpp fileutilities.o networkutilities.o $(FLAGS) $(GTKMM_FLAGS) $(CURL_FLAGS) $(JANSSON_FLAGS) $(LIBZIP_FLAGS) -o launcher
fileutilities.o: src/fileutilities.cpp src/fileutilities.h
	$(CC) src/fileutilities.cpp $(FLAGS) $(GLIBMM_FLAGS) -c -o fileutilities.o
networkutilities.o: src/networkutilities.cpp src/networkutilities.h
		$(CC) src/networkutilities.cpp $(FLAGS) $(GLIBMM_FLAGS) $(CURL_FLAGS) -c -o networkutilities.o
src/launcher-resources.cpp: resources/resources.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies resources/resources.xml)
	$(GLIB_COMPILE_RESOURCES) --generate-source resources/resources.xml --target=src/launcher-resources.cpp
clean:
	-rm launcher src/launcher-resources.cpp fileutilities.o networkutilities.o
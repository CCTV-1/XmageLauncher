CC=g++
FLAGS=-Wall -Wextra -Wpedantic -std=gnu++17 -m64
GLIB_COMPILE_RESOURCES=glib-compile-resources
LIBZIP_FLAGS=$(shell pkg-config --cflags --libs libzip)
CURL_FLAGS=$(shell pkg-config --cflags --libs libcurl)
GTKMM_FLAGS=$(shell pkg-config --cflags --libs gtkmm-3.0)
GLIBMM_FLAGS=$(shell pkg-config --cflags glibmm-2.4)
JANSSON_FLAGS=$(shell pkg-config --cflags --libs jansson)
OS= $(shell uname -o)

ifeq ($(OS),Msys)
	FLAGS += -mwindows
endif

ifndef DEBUG
	FLAGS+=-O3 
endif
ifdef DEBUG
	FLAGS+=-O0 -g3 -pg -DDEBUG
endif

XmageLauncher: src/main.cpp launcher.o
	$(CC) src/main.cpp src/launcher-resources.cpp launcher.o launcherconfig.o utilities.o $(FLAGS) $(GTKMM_FLAGS) $(CURL_FLAGS) $(JANSSON_FLAGS) $(LIBZIP_FLAGS) -o XmageLauncher
launcher.o: src/launcher.cpp src/launcher-resources.cpp launcherconfig.o utilities.o
	$(CC) src/launcher.cpp $(FLAGS) $(GTKMM_FLAGS) -c -o launcher.o
launcherconfig.o: src/launcherconfig.cpp src/launcherconfig.h
	$(CC) src/launcherconfig.cpp $(FLAGS) $(GLIBMM_FLAGS) -c -o launcherconfig.o
utilities.o: src/utilities.cpp src/utilities.h
	$(CC) src/utilities.cpp $(FLAGS) $(GTKMM_FLAGS) -c -o utilities.o
src/launcher-resources.cpp: resources/resources.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies resources/resources.xml)
	$(GLIB_COMPILE_RESOURCES) --generate-source resources/resources.xml --target=src/launcher-resources.cpp
LANGUAGE:po/zh_CN.po po/en_US.po
	msgfmt --output-file=locale/zh_CN/LC_MESSAGES/XmageLauncher.mo po/zh_CN.po
	msgfmt --output-file=locale/en_US/LC_MESSAGES/XmageLauncher.mo po/en_US.po
po/en_US.po: po/XmageLauncher.pot
ifeq ("$(wildcard po/en_US.po)","")
	msginit --input=po/XmageLauncher.pot --locale=en_US --output=po/en_US.po
else
	msgmerge -U po/en_US.po po/XmageLauncher.pot
endif
po/zh_CN.po: po/XmageLauncher.pot
ifeq ("$(wildcard po/zh_CN.po)","")
	msginit --input=po/XmageLauncher.pot --locale=zh_CN --output=po/zh_CN.po
else
	msgmerge -U po/zh_CN.po po/XmageLauncher.pot
endif
po/XmageLauncher.pot: resources/Launcher.ui.h src/launcher.cpp src/utilities.cpp
	xgettext --language=C++ --keyword=_ --keyword=N_ --output=po/XmageLauncher.pot resources/Launcher.ui.h src/launcher.cpp src/utilities.cpp
resources/Launcher.ui.h: resources/Launcher.ui
	intltool-extract --type=gettext/glade resources/Launcher.ui
clean:
	-rm launcher src/launcher-resources.cpp utilities.o resources/Launcher.ui.h\
	po/XmageLauncher.pot po/zh_CN.po po/en_US.po locale/zh_CN/LC_MESSAGES/XmageLauncher.mo locale/en_US/LC_MESSAGES/XmageLauncher.mo
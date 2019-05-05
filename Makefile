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

launcher: src/launcher.cpp src/launcher-resources.cpp launcherconfig.o fileutilities.o networkutilities.o
	$(CC) src/launcher.cpp src/launcher-resources.cpp launcherconfig.o fileutilities.o networkutilities.o $(FLAGS) $(GTKMM_FLAGS) $(CURL_FLAGS) $(JANSSON_FLAGS) $(LIBZIP_FLAGS) -o launcher
launcherconfig.o: src/launcherconfig.cpp src/launcherconfig.h
	$(CC) src/launcherconfig.cpp $(FLAGS) $(GLIBMM_FLAGS) -c -o launcherconfig.o
fileutilities.o: src/fileutilities.cpp src/fileutilities.h
	$(CC) src/fileutilities.cpp $(FLAGS) $(GLIBMM_FLAGS) -c -o fileutilities.o
networkutilities.o: src/networkutilities.cpp src/networkutilities.h	
	$(CC) src/networkutilities.cpp $(FLAGS) $(GLIBMM_FLAGS) $(CURL_FLAGS) -c -o networkutilities.o
src/launcher-resources.cpp: resources/resources.xml $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies resources/resources.xml)
	$(GLIB_COMPILE_RESOURCES) --generate-source resources/resources.xml --target=src/launcher-resources.cpp
LANGUAGE:po/zh_CN.po po/en_US.po
	msgfmt --output-file=locale/zh_CN/LC_MESSAGES/XmageLauncher.mo po/zh_CN.po
	msgfmt --output-file=locale/en_US/LC_MESSAGES/XmageLauncher.mo po/en_US.po
po/en_US.po: po/XmageLauncher.pot
ifeq ("$(wildcard po/en_US.po)","")
	msgmerge -U po/en_US.po po/XmageLauncher.pot
else
	msginit --input=po/XmageLauncher.pot --locale=en_US --output=po/en_US.po
endif
po/zh_CN.po: po/XmageLauncher.pot
ifeq ("$(wildcard po/zh_CN.po)","")
	msgmerge -U po/zh_CN.po po/XmageLauncher.pot
else
	msginit --input=po/XmageLauncher.pot --locale=zh_CN --output=po/zh_CN.po
endif
resources/Launcher.ui.h: resources/Launcher.ui
	intltool-extract --type=gettext/glade resources/Launcher.ui
po/XmageLauncher.pot: resources/Launcher.ui.h src/launcher.cpp
	xgettext --language=C++ --keyword=_ --keyword=N_ --output=po/XmageLauncher.pot resources/Launcher.ui.h src/launcher.cpp
clean:
	-rm launcher src/launcher-resources.cpp fileutilities.o networkutilities.o resources/Launcher.ui.h\
	po/XmageLauncher.pot po/zh_CN.po po/en_US.po locale/zh_CN/LC_MESSAGES/XmageLauncher.mo locale/en_US/LC_MESSAGES/XmageLauncher.mo
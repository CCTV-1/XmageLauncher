bin_PROGRAMS=XmageLauncher

XmageLauncher_SOURCES=launcher.cpp networkutilities.cpp networkutilities.h fileutilities.cpp fileutilities.h launcherconfig.cpp launcherconfig.h launcher-resources.cpp

AM_CPPFLAGS=$(LIBZIP_CFLAGS) $(LIBCURL_CFLAGS) $(GLIBMM_CFLAGS) $(GTKMM_CFLAGS) $(JANSSON_CFLAGS) -DLOCALEDIR=\""$(localedir)"\"
AM_CPPFLAGS+=-std=c++17 -Wall -Wextra -Wpedantic

LDADD=$(LIBZIP_LIBS) $(LIBCURL_LIBS) $(GLIBMM_LIBS) $(GTKMM_LIBS) $(JANSSON_LIBS)
LDADD+=-lstdc++fs

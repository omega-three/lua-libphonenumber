LIB_NAME = lua-libphonenumber
LIB_VER  = 1

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -fPIC

LUA_VERSIONS = 5.4 5.3 5.2 5.1
LUA_VERSION  = $(shell for v in $(LUA_VERSIONS); do \
				pkg-config --exists lua$$v && echo $$v && break; \
			done)

ifeq ($(LUA_VERSION),)
LUA_VERSION = $(shell lua -v 2>&1 | grep -oP 'Lua \K[0-9]\.[0-9]')
endif

ifeq ($(LUA_VERSION),)
LUA_VERSION = 5.1
$(warning Unable to determine Lua version, using 5.1 by default)
endif

ifeq ($(shell pkg-config --exists lua$(LUA_VERSION) && echo yes),yes)
LUA_CFLAGS  = $(shell pkg-config --cflags lua$(LUA_VERSION))
LUA_LDFLAGS = $(shell pkg-config --libs   lua$(LUA_VERSION))
endif

ifeq ($(LUA_CFLAGS),)
LUA_CFLAGS  = -I/usr/include/lua$(LUA_VERSION)
endif

ifeq ($(LUA_LDFLAGS),)
LUA_LDFLAGS = -llua$(LUA_VERSION)
endif

LUA_LIB_DIR = /usr/local/lib/lua/$(LUA_VERSION)

.PHONY: all clean install uninstall

all: $(LIB_NAME)

$(LIB_NAME):
	$(CXX) $(CXXFLAGS) $(LUA_CFLAGS) $(LUA_LDFLAGS) -c $(LIB_NAME).cpp
	$(CXX) $(LUA_CFLAGS) $(LUA_LDFLAGS) -shared -Wl,-soname,$(LIB_NAME).so.$(LIB_VER) -o $(LIB_NAME).so.$(LIB_VER).0 $(LIB_NAME).o -lphonenumber -lgeocoding
	ln -sf $(LIB_NAME).so.$(LIB_VER).0 $(LIB_NAME).so
	ln -sf $(LIB_NAME).so.$(LIB_VER).0 $(LIB_NAME).so.$(LIB_VER)

install:
	@echo "Lua version: $(LUA_VERSION)"
	@echo "Lua lib dir: $(LUA_LIB_DIR)"
	mkdir -p $(LUA_LIB_DIR)
	cp $(LIB_NAME).so* $(LUA_LIB_DIR)/

uninstall:
	rm -f $(LUA_LIB_DIR)/$(LIB_NAME).so*

clean:
	rm -f $(LIB_NAME).o $(LIB_NAME).so*

info:
	@echo "Lua version: $(LUA_VERSION)"
	@echo "Lua lib dir: $(LUA_LIB_DIR)"

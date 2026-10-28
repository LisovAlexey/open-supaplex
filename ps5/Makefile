PS5_PAYLOAD_SDK ?= /opt/ps5-payload-sdk
include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk

TARGET  := OpenSupaplex
ELF     := eboot.elf
DEPS    := build/deps
SDL2_CONFIG := $(PS5_PAYLOAD_SDK)/bin/prospero-sdl2-config

src := $(wildcard ../src/*.c) $(wildcard ../src/sdl2/*.c) $(wildcard ../src/sdl_common/*.c) \
       ../src/lib/ini/ini.c ../src/null/virtualKeyboard.c
obj := $(patsubst ../src/%.c,build/obj/%.o,$(src))

CFLAGS  += -O2 -Wall -DHAVE_SDL2 -I$(DEPS)/include/SDL2 $(shell $(SDL2_CONFIG) --cflags)
# Link our SDL2_mixer by path: the compiler wrapper puts the SDK library dir (with a
# MOD-less SDL2_mixer) before any -L given here
LDFLAGS += $(DEPS)/lib/libSDL2_mixer.a -lmodplug -lc++ $(shell $(SDL2_CONFIG) --libs) -lm

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG=1
endif

.PHONY: all clean package test

all: $(ELF)

$(DEPS)/lib/libSDL2_mixer.a:
	./build-sdl2-mixer.sh

$(obj): $(DEPS)/lib/libSDL2_mixer.a

build/obj/%.o: ../src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

# Folder layout expected by the PS5 homebrew launchers (websrv / Homebrew Launcher):
# /data/homebrew/OpenSupaplex/{eboot.elf,homebrew.js,sce_sys/icon0.png,...}
package: $(ELF)
	rm -rf build/package
	mkdir -p build/package/$(TARGET)
	cp -R ../resources/* build/package/$(TARGET)/
	cp -R assets/* build/package/$(TARGET)/
	cp $(ELF) build/package/$(TARGET)/
	cd build/package && zip -qr ../../$(TARGET)-ps5.zip $(TARGET)

# Send the ELF directly to elfldr: PS5_HOST=ps5 make test
test: $(ELF)
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(or $(PS5_PORT),9021) $^

clean:
	rm -rf build $(ELF) $(TARGET)-ps5.zip

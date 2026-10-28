/*
* This file is part of the OpenSupaplex distribution (https://github.com/sergiou87/open-supaplex).
* Copyright (c) 2020 Sergio Padrino
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../system.h"

#include <errno.h>
#if HAVE_SDL2
#include <SDL.h>
#elif HAVE_SDL
#include <SDL/SDL.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __NDS__
#include <stdbool.h>
#include <filesystem.h>
#endif

#if defined(_3DS)
#include <3ds.h>
#endif

#ifdef __PROSPERO__
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "../globals.h"
#include "../logging.h"

#if defined(__PROSPERO__) && !defined(PS5_APP)
// Homebrew launchers start the game from its own folder, but raw ELF loaders
// don't, so look for the game data in the usual homebrew locations.
static void changeToDataDirectory(void)
{
    struct stat st;
    char path[64];

    if (stat("GFX.DAT", &st) == 0)
    {
        return;
    }

    for (int i = -1; i < 16; ++i)
    {
        if (i < 0)
        {
            snprintf(path, sizeof(path), "/data/homebrew/OpenSupaplex");
        }
        else if (i < 8)
        {
            snprintf(path, sizeof(path), "/mnt/usb%d/homebrew/OpenSupaplex", i);
        }
        else
        {
            snprintf(path, sizeof(path), "/mnt/ext%d/homebrew/OpenSupaplex", i - 8);
        }

        char gfxPath[80];
        snprintf(gfxPath, sizeof(gfxPath), "%s/GFX.DAT", path);

        if (stat(gfxPath, &st) == 0 && chdir(path) == 0)
        {
            spLogInfo("Using game data from %s", path);
            return;
        }
    }

    spLogInfo("Could not find the game data folder");
}
#endif

#if defined(__PROSPERO__) && defined(PS5_APP)
#include <stdlib.h>
#include <sys/mman.h>

int sceKernelAvailableFlexibleMemorySize(size_t *size);

// Native PS5 apps only get a ~13 MB libc heap, which SDL's 8 MB framebuffers
// don't fit in. SDL's allocations go through this allocator instead: big
// blocks are mapped straight from the much larger flexible memory budget.

#define kLargeAllocationThreshold (256 * 1024)
#define kAllocationMagicHeap 0x48454150 // 'HEAP'
#define kAllocationMagicMapped 0x4d415050 // 'MAPP'

typedef struct
{
    size_t size; // requested size
    size_t mappedSize; // 0 for heap blocks
    uint32_t magic;
    uint32_t padding[3];
} AllocationHeader; // 32 bytes, keeps the returned block 32-byte aligned

static void *ps5Malloc(size_t size)
{
    AllocationHeader *header = NULL;

    if (size >= kLargeAllocationThreshold)
    {
        size_t mappedSize = (size + sizeof(AllocationHeader) + 0x3fff) & ~(size_t)0x3fff;
        void *mapped = mmap(NULL, mappedSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        if (mapped == MAP_FAILED)
        {
            spLogInfo("mmap(%zu) failed", mappedSize);
            return NULL;
        }
        header = mapped;
        header->mappedSize = mappedSize;
        header->magic = kAllocationMagicMapped;
    }
    else
    {
        header = malloc(size + sizeof(AllocationHeader));
        if (header == NULL)
        {
            spLogInfo("malloc(%zu) failed", size);
            return NULL;
        }
        header->mappedSize = 0;
        header->magic = kAllocationMagicHeap;
    }

    header->size = size;
    return header + 1;
}

static void ps5Free(void *block)
{
    if (block == NULL)
    {
        return;
    }

    AllocationHeader *header = (AllocationHeader *)block - 1;
    if (header->magic == kAllocationMagicMapped)
    {
        header->magic = 0;
        munmap(header, header->mappedSize);
    }
    else
    {
        header->magic = 0;
        free(header);
    }
}

static void *ps5Calloc(size_t count, size_t size)
{
    size_t total = count * size;
    if (size != 0 && total / size != count)
    {
        return NULL;
    }

    void *block = ps5Malloc(total);
    if (block != NULL)
    {
        memset(block, 0, total); // mapped memory is already zeroed, heap memory isn't
    }
    return block;
}

static void *ps5Realloc(void *block, size_t size)
{
    if (block == NULL)
    {
        return ps5Malloc(size);
    }
    if (size == 0)
    {
        ps5Free(block);
        return NULL;
    }

    AllocationHeader *header = (AllocationHeader *)block - 1;
    if (size <= header->size && (header->magic == kAllocationMagicHeap || size >= kLargeAllocationThreshold))
    {
        header->size = size;
        return block;
    }

    void *newBlock = ps5Malloc(size);
    if (newBlock != NULL)
    {
        memcpy(newBlock, block, header->size < size ? header->size : size);
        ps5Free(block);
    }
    return newBlock;
}

void initializeAppMemory(void)
{
    // Must run before any other SDL call
    SDL_SetMemoryFunctions(ps5Malloc, ps5Calloc, ps5Realloc, ps5Free);
}

#endif

void initializeSystem(void)
{
#if defined(__PROSPERO__) && !defined(PS5_APP)
    changeToDataDirectory();
#endif

#if defined(__PROSPERO__) && defined(PS5_APP)
    size_t flexibleMemory = 0;
    sceKernelAvailableFlexibleMemorySize(&flexibleMemory);
    spLogInfo("Available flexible memory: %zu MB", flexibleMemory / (1024 * 1024));
#endif

    int ret = SDL_Init(SDL_INIT_TIMER);
    if (ret)
    {
        spLogInfo("SDL_Init failed with %d", ret);
        exit(1);
    }

    spLogInfo("SDL_Init succeeded.");

#ifdef __NDS__
    nitroFSInit(NULL);
#endif

#if defined(_3DS)
	if(isOld3DSSystem() == 0)
    {
		osSetSpeedupEnable(true);
        spLogInfo("Using New3DS speed up for better performance");
    }
    romfsInit();
#endif
}

void destroySystem(void)
{
    SDL_Quit();
#if defined(_3DS)
    romfsExit();
#endif
}

uint8_t isOld3DSSystem(void)
{
#if defined(_3DS)
    _Bool isN3DS;
	APT_CheckNew3DS(&isN3DS);
	if (isN3DS == 0)
    {
        return 1;
    }
#endif

    return 0;
}

void exitWithError(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    SDL_Quit();
#if defined(_3DS)
    romfsExit();
#endif
    exit(errno);
}

void int8handler() // proc far        ; DATA XREF: setint8+10o
{
    if (gIsGameRunning != 0)
    {
        gAuxGameSeconds20msAccumulator++;
        if (gAuxGameSeconds20msAccumulator >= 50)
        {
            gAuxGameSeconds20msAccumulator = 0;
            gGameSeconds++;
            if (gGameSeconds >= 60)
            {
                gGameSeconds = 0;
                gGameMinutes++;
                if (gGameMinutes >= 60)
                {
                    gGameMinutes = 0;
                    gGameHours++;
                }
            }
        }
    }

//loc_473B4:              ; CODE XREF: int8handler+4Fj
    // 01ED:0751
    if (gCurrentSoundDuration != 0)
    {
        gCurrentSoundDuration--;
        if (gCurrentSoundDuration == 0)
        {
            gCurrentSoundPriority = 0;
        }
    }
}

void emulateClock()
{
    static Uint32 sLastTickCount = 0;
    static Uint32 sRemainingTicks = 0;
    static const Uint32 sClockInterval = 20; // 20 ms is what the original game seemed to use

    if (sLastTickCount == 0)
    {
        sLastTickCount = getTime();
        return;
    }

    sRemainingTicks += getTime() - sLastTickCount;

    while (sRemainingTicks > sClockInterval)
    {
        int8handler();
        sRemainingTicks -= sClockInterval;
    }

    sLastTickCount = getTime();
}

void handleSystemEvents()
{
    if (gFastMode != FastModeTypeUltra)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                gShouldExitLevel = 1;
                gShouldExitGame = 1;
            }
        }
    }

    emulateClock();
}

uint32_t getTime(void)
{
    return SDL_GetTicks();
}

void waitTime(uint32_t time)
{
    SDL_Delay(time);
}

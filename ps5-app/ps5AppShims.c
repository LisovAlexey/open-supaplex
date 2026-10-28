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

// SDL2 and libiconv need a few libc functions that native PS5 apps can't
// import from the regular system libc. Only the C locale is used.

#include <ctype.h>
#include <stddef.h>
#include <string.h>

typedef int nl_item;

char *nl_langinfo(nl_item item)
{
    (void)item;
    return "UTF-8";
}

size_t ___mb_cur_max(void)
{
    return 1;
}

// SDL2 would otherwise import strcasestr from libScePosixForWebKit, which is
// meant for the browser. build.sh renames SDL2's reference to this function.
char *osp_strcasestr(const char *haystack, const char *needle)
{
    size_t length = strlen(needle);

    for (; *haystack; ++haystack)
    {
        if (strncasecmp(haystack, needle, length) == 0)
        {
            return (char *)haystack;
        }
    }

    return length == 0 ? (char *)haystack : NULL;
}

// SDL2's PS5 keyboard support needs libSceKeyboard and libSceImeDialog, which
// native apps can't load. build.sh points SDL2 to these stubs instead, and SDL
// ignores the errors and runs without a keyboard.

int osp_sceKeyboardInit(void)
{
    return -1;
}

int osp_sceKeyboardOpen(int userId, int type, int index, void *param)
{
    (void)userId;
    (void)type;
    (void)index;
    (void)param;
    return -1;
}

int osp_sceKeyboardReadState(int handle, void *state)
{
    (void)handle;
    (void)state;
    return -1;
}

int osp_sceKeyboardClose(int handle)
{
    (void)handle;
    return 0;
}

int osp_sceImeDialogInit(const void *param, void *extended)
{
    (void)param;
    (void)extended;
    return -1;
}

int osp_sceImeDialogGetStatus(void)
{
    return 0; // SCE_IME_DIALOG_STATUS_NONE
}

int osp_sceImeDialogGetResult(void *result)
{
    (void)result;
    return -1;
}

int osp_sceImeDialogTerm(void)
{
    return 0;
}

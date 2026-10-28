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

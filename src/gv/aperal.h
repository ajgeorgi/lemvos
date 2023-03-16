#ifndef __APERAL__
#define __APERAL__

/* Aperal for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "commen.h"

#ifndef GVCOLOR_FOREGROUND

#define GVCOLOR_FOREGROUND 0xffffffff
#define GVCOLOR_TRANSPARENT 0xefffffff
#define GVCOLOR_BACKGROUND 0xdfffffff

#define _GVCOLOR_ID_MASK 0xff000000
#define _GVCOLOR_INDEXED_MASK ~_GVCOLOR_ID_MASK
#define _GVCOLOR_INDEXED_ID 0xcf000000
#define GVCOLOR_INDEXED(_i) (_GVCOLOR_INDEXED_ID | (_i))

#define GVCOLOR_YELLOW 0xffff00
#define GVCOLOR_RED    0xff0000
#define GVCOLOR_DARK_RED 0x3f0000
#define GVCOLOR_ORANGE  0xC04f4f
#define GVCOLOR_PINK   0xFFC0FF
#define GVCOLOR_WHITE 0xffffff
#define GVCOLOR_BLACK 0x000000
#define GVCOLOR_GREEN 0x00ff00
#define GVCOLOR_BLUE  0x101080
#define GVCOLOR_GREY  0x7f7f7f
#define GVCOLOR_DARK_YELLOW  0xAfAf1f
#define GVCOLOR_LIGHT_GREEN 0x60FF50
#define GVCOLOR_ORANGE_BRIGHT  0xFF6f6f
#define GVCOLOR_LIGHT_GREY 0xE0E0E0
#define GVCOLOR_LIGHT_BLUE 0x3080FF

#endif // GVCOLOR_FOREGROUND

// Color indices and definitions are in "commen.h"
color_type aperalGetColor(int color);
color_type aperalGetIndexedColor(int color, int index);

#endif // __APERAL__
#include <string.h>
#include <stdio.h>

/* Aperal for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/


#include "aperal.h"

#define VIEWER_ORIGINAL_BACKGROUND_COLOR 0x004c60

#define VIEWER_BACKGROUND_COLOR 0x106c9F
#define VIEWER_FOREGROUND_COLOR 0xF0F0F0
#define VIEWER_WIDGET_COLOR 0xCAEFCA
#define VIEWER_WIDGET_DISABLE_COLOR 0xBABABA

typedef struct ColorTableT {
    int use;
    unsigned long color;
} ColorTable;



ColorTable _aperal_standard_color_table[] = {
    { GAPERAL_BACKGROUND, VIEWER_ORIGINAL_BACKGROUND_COLOR },
    { GAPERAL_FOREGROUND, VIEWER_FOREGROUND_COLOR },
    { GAPERAL_WIDGET, VIEWER_WIDGET_COLOR },
    { GAPERAL_WIDGET_DISABLE, VIEWER_WIDGET_DISABLE_COLOR },
    { GAPERAL_WINDOW_BACKGROUND, GVCOLOR_WHITE },
    { GAPERAL_WIDGET_HIGHLIGHT, GVCOLOR_WHITE },
    { GAPERAL_WIDGET_INVERSE_TEXT, GVCOLOR_BLACK },
    { GAPERAL_MODEL, GVCOLOR_YELLOW },
    { GAPERAL_AXIS_X, GVCOLOR_LIGHT_GREEN },
    { GAPERAL_AXIS_Y, GVCOLOR_RED  },
    { GAPERAL_AXIS_Z, GVCOLOR_BLUE },
    { GAPERAL_VERTEX_MARK, GVCOLOR_GREEN },
    { GAPERAL_POLY_START, GVCOLOR_GREY  },
    { GAPERAL_POLY_END, GVCOLOR_BLUE },    
    { GAPERAL_MODEL_OWNER, GVCOLOR_WHITE },
    { GAPERAL_ERROR, GVCOLOR_RED },
    { GAPERAL_CHOICE_BUTTON_HIGHLIGHT, GVCOLOR_WHITE },
    { GAPERAL_VERTEX_SELECTED,GVCOLOR_PINK },
    { GAPERAL_POLY_SELECTED, GVCOLOR_PINK },
    { GAPERAL_SOLID_SELECTED, GVCOLOR_PINK },
    { GAPERAL_NORMALE, GVCOLOR_PINK },
    { GAPERAL_TRI, GVCOLOR_ORANGE_BRIGHT },
    { GAPERAL_INPUT_BACKGROUND, GVCOLOR_LIGHT_GREY },
    { GAPERAL_INPUT_FRAME, GVCOLOR_PINK },
    { GAPERAL_INPUT_FOREGROUND, GVCOLOR_BLACK },
    { GAPERAL_CURSOR, GVCOLOR_ORANGE },
    { GAPERAL_INPUT_FOCUS, GVCOLOR_RED },
    { GAPERAL_POPUP_BACKGROUND, VIEWER_BACKGROUND_COLOR },
    { GAPERAL_FAILED, GVCOLOR_RED },
    { GAPERAL_BOUNDING_BOX, GVCOLOR_GREEN },
    { GAPERAL_GRID, GVCOLOR_LIGHT_BLUE },
    { GAPERAL_MESH, GVCOLOR_GREEN },
    { GAPERAL_MESH_SELECTED, GVCOLOR_LIGHT_GREEN },
    { GAPERAL_TEXT, VIEWER_WIDGET_COLOR },
    { GAPERAL_VERTEX_MARK2, GVCOLOR_WHITE },
    { GAPERAL_VERTEX_MARKERROR, GVCOLOR_RED },
    { GAPERAL_MEASUREMENT, GVCOLOR_GREY },
    { GAPERAL_CENTER_POINT, GVCOLOR_DARK_RED },
    { GAPERAL_GRAVITY_MEA, GVCOLOR_ORANGE },
    { GAPERAL_BOUYANCY_MEA, GVCOLOR_BLUE },
    { GAPERAL_OCEAN, GVCOLOR_LIGHT_BLUE },
    { GAPERAL_TEXT_HIGHLIGHT, GVCOLOR_RED }
};


static ColorTable *_aperal_color_table = _aperal_standard_color_table;
static const int _aperal_color_table_size = sizeof(_aperal_standard_color_table)/sizeof(_aperal_standard_color_table[0]);

int aperalInit()
{
    // memset(_aperal_color_index,0,sizeof(_aperal_color_index));
        
    return 0;
}

color_type aperalGetColor(int color)
{
    if ((0 < color) && (_aperal_color_table_size >= color))
    {
        // printf("Color table size = %zi, color = %i (%lX)\n",_aperal_color_table_size,color,_aperal_color_table[color-1].color);
        
        return _aperal_color_table[color-1].color;
    }

    return GVCOLOR_WHITE;
}

static int _aperalTransformColor(color_type *color, int index)
{
    if (5*index < 0xff)
    {
        color_type r = ((*color) >> 16) & 0xff;
        color_type g = ((*color) >> 8) & 0xff;
        color_type b = ((*color)) & 0xff;
        
        unsigned int shift =  5*index;        
        
        if (r < (0xff-shift))
        {
            r += shift;
        }
        if (g < (0xff-shift))
        {
            g += shift;
        }
        if (b < (0xff-shift))
        {
            b += shift;
        }
            
        *color = (r<<16) | (g<<8) | b;
        
        return 0;
    }
    
    return -1;
}

color_type aperalGetIndexedColor(int color, int index)
{
    if ((0 < color) && (_aperal_color_table_size >= color) && (index >= 0))
    {
        color_type col = aperalGetColor(color);

        _aperalTransformColor(&col,index);
        
        return col;
    }

    return GVCOLOR_WHITE;    
}

color_type aperalHighlightingColor(color_type color)
{
    if (GVCOLOR_FOREGROUND == color)
    {
        return aperalGetColor(GAPERAL_WIDGET_HIGHLIGHT);
    }
 
    color_type g = ((color) >> 8) & 0xff;
    color_type b = ((color)) & 0xff;

    g /= 2;
    b /= 2;
    
    return 0xff0000 | (g << 8) | b;
}

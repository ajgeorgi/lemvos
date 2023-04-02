#ifndef __TABLE__
#define __TABLE__

/* Table for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "widget.h"

#define TABLE_MAX_CELL_TEXT 100

#define TABLE_FLAG_COLUMN_WIDTHSETTING (1<<0)
#define TABLE_FLAG_COLOR_CHANGED  (1<<1)
#define TABLE_FLAG_SEPERATOR  (1<<2)

typedef struct TableCellT {
    char content[TABLE_MAX_CELL_TEXT];
    int font_size;
    int font_baseline;
    unsigned int flags;
    color_type color;
    ViewerWidget *widget;
    int height;
    int y;
} TableCell;

typedef struct TableColumnT {
    char name[16];
    int width;
    int x;
    unsigned int flags;
    TableCell cells[];
} TableColumn;


typedef struct TableWidgetT {
    MagicNumber magic;
    ObjectType type;
    IndexType  index;
    IndexType child_count;
    flag_type flags;    
    struct GObjectT *parent;
    struct GObjectT *next;
//-------------------------------------    
    ViewerWidget *widget;
    int number_of_columns;
    int number_of_rows;
    int max_number_of_columns;
    int max_number_of_rows;
    int width;
    int height;
    int x;
    int y;
    TableColumn columns[]; // Do not access! We don't like the compiler addressing here!
} TableWidget;

TableWidget *tableCreateTable(ViewerWidget *parent, const char* name, int columns, int rows);

#define TABLE_APPEND_ROW (-1)

int tableAddTextToCell(TableWidget *table, int column, int row, const char* content);
int tableAddWidgetToCell(TableWidget *table, int column, int row, ViewerWidget *widget);
ViewerWidget *tableGetWidgetFromCell(TableWidget *table, int column, int row);

int tablePosition(TableWidget *table, int x, int y);
// int tableSize(TableWidget *table, int width, int height);

int tableClear(TableWidget *table);

int tableGetColumnWidth(TableWidget *table, int column);
int tableGetNumberOfRows(TableWidget *table);

int tableSetColumnWidth(TableWidget *table, int column, int width);
int tableSetCellFontSize(TableWidget *table, int column, int row, int font_size);
int tableSetCellColor(TableWidget *table, int column, int row, color_type font_color);

int tableSetRowSeperator(TableWidget *table, int row);


#endif // __TABLE__
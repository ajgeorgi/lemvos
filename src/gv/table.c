#include <string.h>
#include <stdio.h>

/* Table for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "table.h"
#include "viewer.h"

static int tableViewerCallback(ViewerWidget *w, int ev, int x, int y, void *data);

TableWidget *tableCreateTable(ViewerWidget *parent, const char* name, int columns, int rows)
{
    (void)name;
    const int size_of_column = sizeof(TableColumn) + rows*sizeof(TableCell);    
    const int size_of_table = sizeof(TableWidget) + size_of_column*columns;
    TableWidget *table = (TableWidget*)memory_aloc(size_of_table );
    memset(table,0,size_of_table);
    
    objectInit((GObject *)table, (GObject *)parent, 0, OBJ_TABLE);

    table->widget = widgetCreateWidget(parent,NULL,VIEWERWIDGET_TYPE_WINDOW, 0, 0, 10, 10, tableViewerCallback, table);
    table->max_number_of_columns = columns;
    table->max_number_of_rows = rows;

#ifdef _DEBUG_TABLE    
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("Created table [%s] with size (%i,%i) (%i bytes)\n",
        vertexPath((GObject*)table,s1,sizeof(s1)),
        table->max_number_of_columns,table->max_number_of_rows,size_of_table);
    
    
    TableColumn *tcolumn = (TableColumn *) (((char*)&table->columns[0]) + (columns-1)*size_of_column);
    TableCell *trow = &tcolumn->cells[rows-1];

    char *tp = ((char*)trow) + sizeof(TableCell);
    
    if ((tp-((char*)table)) != size_of_table)
    {
        FATAL1("Memory does not match here!\n");
    }
        
    LOG("[%s]: mem start = %p, end = %p, |end-start| = %li, size = %i\n",
        vertexPath((GObject*)table,s1,sizeof(s1)),
        (char*)table,tp,(tp-((char*)table)),size_of_table);

    LOG("[%s]: sizeof(TableWidget) = %li, sizeof(TableColumn) = %li, sizeof(TableCell) = %li, \n",
        vertexPath((GObject*)table,s1,sizeof(s1)),
        sizeof(TableWidget),sizeof(TableColumn),sizeof(TableCell));
#endif
    
    return table;
}

TableColumn *tableGetColumn(TableWidget *table, int column)
{
   const int size_of_column = sizeof(TableColumn) + table->max_number_of_rows*sizeof(TableCell);    
     
   TableColumn *tcolumn = (TableColumn *) (((char*)&table->columns[0]) + column*size_of_column);
   
#ifdef _DEBUG   
    const int size_of_table = sizeof(TableWidget) + size_of_column*table->max_number_of_columns;
   
    TableColumn *last_column = (TableColumn *) (((char*)&table->columns[0]) + (table->max_number_of_columns-1)*size_of_column);
    TableCell *last_row = &last_column->cells[table->max_number_of_rows-1];

    char *tp = ((char*)last_row) + sizeof(TableCell);
    
    if ((tp-((char*)table)) != size_of_table)
    {
        FATAL1("Memory does not match here!\n");
    }

    if (((char*)tcolumn) >= tp)
    {
        FATAL1("Memory for column does not match!\n");
    }
#endif

   return tcolumn;
}

int _tableSizeUpdate(TableWidget *table)
{
    int pos_x = 1;
    int pos_y = 1;
    int width = 0;
    int height = 0;
    
    int font_baseline = 0;
    int font_height = 0;
    viewerStringSize("j", 
            1, 
            NULL, 
            NULL, 
            NULL, 
            &font_height, 
            &font_baseline);     

    char s1[GM_VERTEX_BUFFER_SIZE];
    
#ifdef _DEBUG_TABLE                                                            
    char s2[GM_VERTEX_BUFFER_SIZE];
#endif
        

    for (int column = 0 ; column < table->number_of_columns; column++)
    {
        TableColumn *tcolumn = tableGetColumn(table, column);
        
        if (0 == (tcolumn->flags & TABLE_FLAG_COLUMN_WIDTHSETTING))
        {
            tcolumn->width = 0;
        }
        
        tcolumn->x = pos_x;
        pos_y = 0;
    
        int max_column_width = 0;        
        int max_row_height = 0;
        for (int row = 0; row < table->number_of_rows; row++)
        {
            TableCell *trow = &tcolumn->cells[row];                       
            
            if (0 == tcolumn->width)
            {
                int twidth = 0;
                if (trow->widget)
                {
                    twidth = trow->widget->width;
                }
                else
                {
                    int normal_font_size = -1;
                    if (0 < trow->font_size)
                    {
                        normal_font_size = viewerGetFontSize();
                        if (normal_font_size == trow->font_size)
                        {
                            normal_font_size = 0;                        
                        }                        
                        viewerSetFontSize(trow->font_size);
                        
                        trow->height = 0;
                        max_row_height = 0;
                    }
                    viewerStringSize(trow->content, 
                        strlen(trow->content), 
                        &twidth, 
                        NULL, 
                        NULL, 
                        &font_height, 
                        &font_baseline);    
                    
                    if (0 < normal_font_size)
                    {
                        viewerSetFontSize(normal_font_size);
                    }
                }
                
                if (twidth > max_column_width)
                {
                    max_column_width = twidth;
                }
            }
            
            if (0 == trow->height)
            {
                if (trow->widget)
                {
                    trow->height = trow->widget->height + 2;
                }
                else
                {
                    trow->height = font_height + 1;
                }
            }
               
               
            if (trow->widget)
            {
                trow->y = pos_y;                   
            }
            else
            {
                trow->y = pos_y;
                trow->font_baseline = font_baseline;
            }

            pos_y += trow->height;                                    

            if (max_row_height < trow->height)
            {
                max_row_height = trow->height;
            }
            
            if (table->number_of_columns == column + 1)
            {
                height += max_row_height;
            }
#ifdef _DEBUG_TABLE                                                        
            LOG("%s:Cell (%i,%i) at (%i,%i) cwidth=%i, #col = %i, [%s]=(%i,%i) at (%i,%i)\n",
                vertexPath((GObject*)table,s1,sizeof(s1)),
                column,row, tcolumn->x, trow->y,tcolumn->width, table->number_of_columns,
                vertexPath((GObject*)trow->widget,s2,sizeof(s2)),
                trow->widget?trow->widget->width:-1,trow->widget?trow->widget->height:-1,
                trow->widget?trow->widget->x:-1,trow->widget?trow->widget->y:-1
               );
#endif            
        }  
        
        if (max_column_width > tcolumn->width)
        {
            if (0 == (tcolumn->flags & TABLE_FLAG_COLUMN_WIDTHSETTING))
            {
                tcolumn->width = max_column_width + 2;
            }
#ifdef _DEBUG_TABLE                                                        
            LOG("New column %i width now %i\n",column, tcolumn->width);
#endif            
        }  
        
        pos_x += tcolumn->width;
        width += tcolumn->width;
    } 
    
    
    for (int column = 0 ; column < table->number_of_columns; column++)
    {
        TableColumn *tcolumn = tableGetColumn(table, column);
        
        for (int row = 0; row < table->number_of_rows; row++)
        {
            TableCell *trow = &tcolumn->cells[row];                       

            if (trow->widget)
            {
                trow->widget->flags |= (VIEWERWIDGET_FLAG_NOFRAME|VIEWERWIDGET_FLAG_LEFT_ALIGN);

                if (widgetSizeWidget(trow->widget, tcolumn->width - 3, trow->height - 2))
                {
                    ERROR("Failed to set width on [%s]\n",vertexPath((GObject*)trow->widget,s1,sizeof(s1)));
                }
                widgetMoveWidget(trow->widget, tcolumn->x + 1, trow->y);
            }
            
        }
    }
    table->width = width;
    table->height = height;

#ifdef _DEBUG_TABLE                                                
    LOG("sizeof [%s] = (%i,%i)\n",
        vertexPath((GObject*)table,s1,sizeof(s1)),
        table->width ,table->height);
#endif    
    
    widgetSizeWidget(table->widget, table->width, table->height);

    return 0;
}

int tableSetColumnWidth(TableWidget *table, int column, int width)
{
    if ((column < table->max_number_of_columns) && (0 <= column))        
    {
        TableColumn *tcolumn = tableGetColumn(table, column);
        tcolumn->width = width;
        tcolumn->flags |= TABLE_FLAG_COLUMN_WIDTHSETTING;
        
        if (column >= table->number_of_columns)
        {
            table->number_of_columns = column + 1;
        }
#ifdef _DEBUG_TABLE                                            
        char s1[GM_VERTEX_BUFFER_SIZE];
        LOG("%s: Setting column %i to width = %i\n",vertexPath((GObject*)table,s1,sizeof(s1)),column,tcolumn->width );
#endif        
        return 0;
    }
    
    ERROR("Can not set colum width of \"%s\"(%i) to %i\n",table->widget->name,column,width);
    
    return -1;    
}

int tableSetCellFontSize(TableWidget *table, int column, int row, int font_size)
{
    if ((column < table->number_of_columns) && (0 <= column))        
    {
        TableColumn *tcolumn = tableGetColumn(table, column);
        if ((row >= 0) && (row < table->number_of_rows))
        {
            TableCell *trow = &tcolumn->cells[row];
            
            trow->font_size = font_size;
            
            return 0;
        }        
    }    
    
    return 1;
}


int tablePosition(TableWidget *table, int x, int y)
{        
    table->x = x;
    table->y = y;

    widgetMoveWidget(table->widget,x,y);    
    
    _tableSizeUpdate(table);
    
#ifdef _DEBUG_TABLE                                    
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("tablePosition: Table [%s] size=(%i,%i), pos=(%i,%i)\n",
        vertexPath((GObject*)table,s1,sizeof(s1)),
        table->width, table->height,
        x,y);
#endif    
    
    // widgetSizeWidget(table->widget, table->width, table->height);
    
    return 0;
}

int tableGetColumnWidth(TableWidget *table, int column)
{
    if ((column < table->max_number_of_columns) && (0 <= column))        
    {
        TableColumn *tcolumn = tableGetColumn(table, column);
        
        return tcolumn->width;
    }
    
    ERROR("Not a column %i on \"%s\"\n",column,table->widget->name);
    
    return 0;
}

int tableGetNumberOfRows(TableWidget *table)
{
    return table->number_of_rows;
}

int tableAddTextToCell(TableWidget *table, int column, int row, const char* content)
{    
    if ((column < table->max_number_of_columns) && (column >= 0))
    {
        TableColumn *tcolumn = tableGetColumn(table, column);

        if (TABLE_APPEND_ROW == row)
        {
            row = table->number_of_rows;
        }
        
        if (column >= table->number_of_columns)
        {
            table->number_of_columns = column + 1;
        }
        
                
        if ((row >= 0) && (row < table->max_number_of_rows))
        {
            TableCell *trow = &tcolumn->cells[row];
                        
            strncpy(trow->content,content,sizeof(trow->content));
            trow->content[sizeof(trow->content)-1] = 0;
            
            if (row >= table->number_of_rows)
            {
                table->number_of_rows = row + 1;
            }
        
            _tableSizeUpdate(table);

            return 0;            
        }
    }
    
    ERROR("Can not add content to (%i,%i) of \"%s\"\n",column,row,table->widget->name);

    return -1;    
}

int tableAddWidgetToCell(TableWidget *table, int column, int row, ViewerWidget *widget)
{   
    if (widget && table)
    {
        if ((column < table->max_number_of_columns) && (column >= 0))
        {
            TableColumn *tcolumn = tableGetColumn(table, column);

            if (TABLE_APPEND_ROW == row)
            {
                row = table->number_of_rows;
            }
            
            if (column >= table->number_of_columns)
            {
                table->number_of_columns = column + 1;
            }
            
            if ((row >= 0) && (row < table->max_number_of_rows))
            {
                TableCell *trow = &tcolumn->cells[row];
                trow->widget = widget;
                
                if (TABLE_FLAG_COLOR_CHANGED & trow->flags)
                {
                    widgetSetColor(widget, trow->color, trow->color);
                }
                
                widget->flags |= (VIEWERWIDGET_FLAG_NOFRAME|VIEWERWIDGET_FLAG_LEFT_ALIGN);
                if (row >= table->number_of_rows)
                {
                    table->number_of_rows = row + 1;
                }
                
                _tableSizeUpdate(table);
                
                return 0;            
            }
        }
    }
    
    char s1[GM_VERTEX_BUFFER_SIZE];
    char s2[GM_VERTEX_BUFFER_SIZE];
    ERROR("Can not add widget [%s] \"%s\" at (%i,%i) to [%s]\n",
          vertexPath((GObject*)widget,s1,sizeof(s1)),
          table->widget?table->widget->name:UNKNOWN_SYMBOL,
          column,row,          
          vertexPath((GObject*)table,s2,sizeof(s2))          
         );
    
    return -1;
}

static int _tablePaint(TableWidget *table)
{
    const int normal_font_size = viewerGetFontSize();
    
    const color_type  normal_color = viewerGetDrawingColor();

    for (int column = 0 ; column < table->number_of_columns; column++)
    {
        TableColumn *tcolumn = tableGetColumn(table, column);

        for (int row = 0; row < table->number_of_rows; row++)
        {
            TableCell *trow = &tcolumn->cells[row];

            if (0 == column)
            {
                if (0 == (TABLE_FLAG_SEPERATOR & trow->flags))
                {
                    viewerSetLineWidth(1);
                }
                
                const int y = table->widget->y + trow->y - 1;
                viewerDrawLine(table->widget->x, y, 
                            table->widget->x + table->width, y);
                
                if (0 == (TABLE_FLAG_SEPERATOR & trow->flags))
                {                
                    viewerSetLineWidth(-1);
                }
            }
            
            if (NULL == trow->widget)
            {
#ifdef _DEBUG_TABLE_X
                LOG("Drawing Text [%i,%i]=\"%s\" at (%i,%i)\n",column,row,trow->content, tcolumn->x, trow->y);
#endif          
                
                if (0 < trow->font_size)
                {                    
                    viewerSetFontSize(trow->font_size);
                }                
                
                if (TABLE_FLAG_COLOR_CHANGED & trow->flags)
                {
                    viewerSetDrawingColor(trow->color);
                }
                
                widgetDrawTextWidget(table->widget, tcolumn->x, trow->y  + trow->font_baseline, trow->content, strlen(trow->content));                                

                if (TABLE_FLAG_COLOR_CHANGED & trow->flags)
                {                
                    viewerSetDrawingColor(normal_color);
                }
                
                if (normal_font_size != trow->font_size)
                {
                    viewerSetFontSize(normal_font_size);
                }
            }
        }
    }  
    
    if (0 < normal_font_size)
    {
        viewerSetFontSize(normal_font_size);
    }
    
    return 0;
}

int tableViewerCallback(ViewerWidget *w, int ev, int x, int y, void *data)
{
    (void)w;
    (void)ev;
    (void)x;
    (void)y;
    (void)data;
    
#ifdef _DEBUG_TABLE_X                                   
    char s1[GM_VERTEX_BUFFER_SIZE];
    LOG("TB:%s:%s:EV(%s):(%i,%i)\n",vertexPath((GObject*)w,s1,sizeof(s1)),w->name?w->name:"?",widgetEventToText(ev),x,y);
#endif    
    
    int ret = 0;
    switch(ev)
    {
        case VIEWEREVENT_EXPOSURE: {
                TableWidget *table = (TableWidget *)isGObject(OBJ_TABLE,data);   
                if (table)
                {
                    _tablePaint(table);
                    ret = 1;
                }
                break;
        }
        case VIEWEREVENT_RESIZE: {
                TableWidget *table = (TableWidget *)isGObject(OBJ_TABLE,data);   
                if (table)
                {
                    _tableSizeUpdate(table);
                    ret = 1;
                }
                break;
        }
    }
    
    return ret;
}

int tableClear(TableWidget *table)
{
    for (int column = 0; column < table->number_of_columns; column++)
    {        
        TableColumn *tcolumn = tableGetColumn(table, column);
        
        tcolumn->flags = 0;  
        tcolumn->width = 0;
        for (int row = 0; row < table->number_of_rows; row++)
        {
            TableCell *trow = &tcolumn->cells[row];
            memset(trow->content,0,sizeof(trow->content));
            
            if (trow->widget)
            {
                widgetSetTextPosition(trow->widget, -1,-1);
                
               trow->widget->flags &= ~(VIEWERWIDGET_FLAG_HIGHLIGHTED|VIEWERWIDGET_FLAG_NOFRAME|VIEWERWIDGET_FLAG_LEFT_ALIGN);
            }

            trow->widget = NULL;
            trow->font_size = 0;
            trow->color = 0;
            trow->flags = 0;                            
            trow->height = 0;
        } 
    }

    table->number_of_rows = 0;
    table->height = 0;
    table->width = 0;

    return 0;    
}

ViewerWidget *tableGetWidgetFromCell(TableWidget *table, int column, int row)
{
    if ((column < table->max_number_of_columns) && (column >= 0))
    {
        TableColumn *tcolumn = tableGetColumn(table, column);

        if (TABLE_APPEND_ROW == row)
        {
            row = table->number_of_rows - 1;
        }
        
        if ((row >= 0) && (row < table->max_number_of_rows))
        {
            TableCell *trow = &tcolumn->cells[row];
            
            return trow->widget;
        }            
    }
    
    return NULL;
}

int tableSetCellColor(TableWidget *table, int column, int row, color_type font_color)
{
    if ((column < table->number_of_columns) && (0 <= column))        
    {
        TableColumn *tcolumn = tableGetColumn(table, column);

        if (TABLE_APPEND_ROW == row)
        {
            row = table->number_of_rows - 1;
        }
        
        if (tcolumn && (row >= 0) && (row < table->number_of_rows))
        {
            TableCell *trow = &tcolumn->cells[row];
            
            if (GVCOLOR_TRANSPARENT == font_color)
            {
                trow->color = 0;
                trow->flags &= ~TABLE_FLAG_COLOR_CHANGED;                

                if (trow->widget)
                {
                    widgetSetColor(trow->widget,GVCOLOR_TRANSPARENT,GVCOLOR_TRANSPARENT);
                }
            }
            else
            {
                trow->color = font_color;
                trow->flags |= TABLE_FLAG_COLOR_CHANGED;

                if (trow->widget)
                {
                    widgetSetColor(trow->widget,font_color,font_color);
                }                                
            }

            
            return 0;
        }        
    }    
    
    return 1;    
}

int tableSetRowSeperator(TableWidget *table, int row)
{
    TableColumn *tcolumn = tableGetColumn(table, 0);
    if (tcolumn && (row >= 0) && (row < table->number_of_rows))
    {
        TableCell *trow = &tcolumn->cells[row];
        trow->flags |= TABLE_FLAG_SEPERATOR;
        
        return 0;
    }
    return -1;
}
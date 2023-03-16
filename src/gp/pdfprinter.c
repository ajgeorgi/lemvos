
#include <stdio.h>
#include <string.h>
#include <time.h>

/* PDFPrinter for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/



/*

This file uses libHaru.

*
* License
*
Haru is distributed under the ZLIB/LIBPNG License. Because ZLIB/LIBPNG License 
is one of the freest licenses, You can use Haru for various purposes.

The license of Haru is as follows.

Copyright (C) 1999-2006 Takeshi Kanno
Copyright (C) 2007-2009 Antony Dovgal

This software is provided 'as-is', without any express or implied warranty.

In no event will the authors be held liable for any damages arising from the 
use of this software.

Permission is granted to anyone to use this software for any purpose,including 
commercial applications, and to alter it and redistribute it freely, subject 
to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not claim 
    that you wrote the original software. If you use this software in a 
    product, an acknowledgment in the product documentation would be 
    appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be 
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.


*/

#include "configuration.h"

#include "hpdf.h"
#include "pdfprinter.h"
#include "hpdf_types.h"

#define PDF_EXTENSION ".pdf"

static int _pdfprinterPrintSolid(Solid *solid, HPDF_Page printer_drawing);
// static int _pdfprinterPrintPolygon(Polygon *poly, HPDF_Page printer_drawing);
static HPDF_Doc _printer_pdf = NULL;
static int _pdf_failed = 0;
static VertexProjection _pdf_projection = NULL;
static int _pdfprinter_vertexCount = 0;
static CommonErrorHandler _printer_error_handler = NULL;
static int _pdf_line_width = 1;

void printer_Error_Handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    (void)user_data;
    char textBuffer[100];
    snprintf(textBuffer,sizeof(textBuffer),"PDF: Failed! error_no = %lX, detail_no = %lX\n",error_no,detail_no);
    
    if (_printer_error_handler)
    {
        _printer_error_handler(0,textBuffer);
    }
    
    _pdf_failed = 1;
}


int pdfprinterInit(VertexProjection projection, CommonErrorHandler error_handler)
{
    _pdf_projection = projection;
    _printer_error_handler = error_handler;
    _pdf_line_width = configInt(configGetConfig("pdf.line_width"),_pdf_line_width);

    return 0;
}

static HPDF_Page _createPage()
{
    HPDF_Page printer_drawing;

    /* set page mode to use outlines. */
    printer_drawing = HPDF_AddPage (_printer_pdf);

    _pdf_line_width = configInt(configGetConfig("pdf.line_width"),_pdf_line_width);
    
    HPDF_Page_SetSize (printer_drawing, HPDF_PAGE_SIZE_A3, HPDF_PAGE_LANDSCAPE);

    HPDF_Page_SetLineWidth (printer_drawing,_pdf_line_width);
    HPDF_Page_SetLineJoin (printer_drawing,HPDF_ROUND_JOIN);
    HPDF_Page_SetLineCap (printer_drawing,HPDF_ROUND_END);

    return printer_drawing;
}

int pdfprinterGetVertexCount()
{
    return _pdfprinter_vertexCount;
}


int pdfprinterPrintModel(Model *model, const char* fileName)
{
    if (NULL == model)
    {
        ERROR1("No model. Nothing to print!\n");
        return -1;
    }
        
    _printer_pdf = HPDF_New (printer_Error_Handler, NULL);
        
    if (NULL != _printer_pdf)
    {    
        _pdfprinter_vertexCount = 0;
        
        HPDF_SetPageMode (_printer_pdf,  HPDF_PAGE_MODE_USE_NONE );

        HPDF_SetInfoAttr (_printer_pdf, HPDF_INFO_AUTHOR, model->owner);
        HPDF_SetInfoAttr (_printer_pdf, HPDF_INFO_CREATOR, "GP of Georgi Bootbau");        
        HPDF_SetInfoAttr (_printer_pdf, HPDF_INFO_TITLE, commenNameFromModel(model->name));

        // HPDF_SetPassword (_printer_pdf,"owner","user");
        HPDF_SetPassword (_printer_pdf,":3xy2z_.Q,1||23$yXq",NULL); // ???

        HPDF_SetEncryptionMode (_printer_pdf, HPDF_ENCRYPT_R3, 16);
 
        if (commenIsTime(&model->last_modified))
        {
            HPDF_Date pdf_date;
            
            memset(&pdf_date,0,sizeof(pdf_date));

            pdf_date.year = model->last_modified.tm_year + 1900;
            pdf_date.month = model->last_modified.tm_mon + 1;
            pdf_date.day = model->last_modified.tm_mday;
            pdf_date.hour = model->last_modified.tm_hour;
            pdf_date.minutes = model->last_modified.tm_min;
            pdf_date.ind = '+';
            pdf_date.off_hour = 1;

            LOG("modification time in PDF: %s\n",asctime(&model->last_modified));

            HPDF_SetInfoDateAttr (_printer_pdf,  HPDF_INFO_MOD_DATE, pdf_date);
        }

        if (commenIsTime(&model->created))
        {
            HPDF_Date pdf_date;
            
            memset(&pdf_date,0,sizeof(pdf_date));
            
            pdf_date.year = model->created.tm_year + 1900;
            pdf_date.month = model->created.tm_mon + 1;
            pdf_date.day = model->created.tm_mday;
            pdf_date.hour = model->created.tm_hour;
            pdf_date.minutes = model->created.tm_min;
            pdf_date.ind = '+';
            pdf_date.off_hour = 1;
            
            LOG("creation time in PDF: %s\n",asctime(&model->created));

            HPDF_SetInfoDateAttr (_printer_pdf,  HPDF_INFO_CREATION_DATE, pdf_date);
        }
        
        HPDF_SetPermission (_printer_pdf, HPDF_ENABLE_PRINT | HPDF_ENABLE_READ );
     
        HPDF_Page printer_drawing = _createPage();
        
        for (Solid *solid = (Solid*)isCObject(OBJ_SOLID,model->first); NULL != solid; solid = (Solid*)isCObject(OBJ_SOLID,solid->next))
        {     
            _pdfprinterPrintSolid(solid,printer_drawing);
        }
        
        if (0 == _pdf_failed)
        {
            remove(fileName);
            HPDF_SaveToFile (_printer_pdf, fileName);
        }
        else
        {
            ERROR1("No PDF generated.\n");
        }

        HPDF_Free (_printer_pdf);
    }
    else
    {
        ERROR("Failed to open \"%s\"\n",fileName);
        perror("Unable to open file.\n");
    }
    
    _printer_pdf = NULL;
    
    return _pdf_failed;
}

static int _pdfprinterPrintPolygonRelative(Polygon *poly, HPDF_Page printer_drawing)
{
    if (poly && _pdf_projection && printer_drawing)
    {                           
        // char textbuffer[30],textbuffer2[30];
        EPoint pi[GM_VERTEX_DIMENSION]; 
        
        //fprintf(stderr,"first x = %s, p = %s\n",EPoint3ToString(pi_old,textbuffer,sizeof(textbuffer)),EPoint3ToString(pi_old,textbuffer2,sizeof(textbuffer2)));

        pi[0] = poly->p[0];
        pi[1] = poly->p[1];
        pi[2] = poly->p[2];

        Solid * solid = (Solid*)isObject(poly->parent);
        
        if (solid)
        {
            EPoint *solid_p = solid->p;
        
            pi[0] += solid_p[0];
            pi[1] += solid_p[1];
            pi[2] += solid_p[2];
        }
        // AddEPoints(pi_old,poly->first->x,poly->p);
        
        int x0[2];
        _pdf_projection(pi,x0);

        HPDF_Page_SetLineWidth (printer_drawing, _pdf_line_width);

        HPDF_Page_MoveTo(printer_drawing,x0[0],x0[1]);

        EPoint pi_old[3];
        CopyEPoint(pi_old,pi);
        for (const Vertex *vertex = poly->first; NULL != vertex; vertex = (Vertex*)vertex->next)
        {
            AddEPoints(pi,vertex->x,pi_old);
         
            // Draw the plot
            
            int x1[2];
            _pdf_projection(pi,x1);
            
            HPDF_Page_LineTo( printer_drawing, x1[0], x1[1]);
                        
            CopyEPoint(pi_old,pi);
            _pdfprinter_vertexCount++;
        }
        HPDF_Page_Stroke (printer_drawing);
        

        // HPDF_Page_EndPath (printer_drawing);

        // fprintf(stderr,"Vertices plotted %i\n",vertexCount);        
    }
   
    return 0;
}

static int _pdfprinterPrintPolygonAbsolut(Polygon *poly, HPDF_Page printer_drawing)
{
    if (poly && (poly->flags & GM_GOBJECT_FLAG_POLY_ABSOLUT))
    {    
       // Solid *solid = (Solid*)isCObject(OBJ_SOLID,poly->parent);

        const Vertex *v0 = (Vertex*)poly->first;
        int x0[2];
        _pdf_projection(v0->x,x0);
        
        HPDF_Page_SetLineWidth (printer_drawing, _pdf_line_width);
        HPDF_Page_MoveTo(printer_drawing,x0[0],x0[1]);

        for (const Vertex *vertex = (Vertex*)v0->next; NULL != vertex; vertex = (Vertex*)vertex->next)
        {
            // Draw the plot
            
            int x1[2];
            _pdf_projection(vertex->x,x1);
            
            HPDF_Page_LineTo( printer_drawing, x1[0], x1[1]);  

/*            
            if (GM_SOLID_SHOW_TRI & solid->flags)
            {
                viewerSetDrawingColor(aperalGetColor(GAPERAL_TRI));
                for (int i = 0; i < vertex->number_of_connections; i++)
                {
                    Vertex *vert2 = vertex->connections[i];
                    
                    if (vert2)
                    {
                       viewerDrawLine3D(vertex->x, vert2->x);
                    }
                }
            }
            */
            _pdfprinter_vertexCount++;
        }
        HPDF_Page_Stroke (printer_drawing);       
    }
   
    return 0;
}

int _pdfprinterPrintMaterial(Material *material)
{
    (void)material;
 //  fprintf(_printerFile,"Material(\"%s\");%g;%lX;%g\n",material->name,material->density,material->color,material->thick);
   
   return 0;
}

int _pdfprinterPrintSolid(Solid *solid, HPDF_Page printer_drawing)
{
    if (NULL == solid)
    {
       return -1;
    }

    if (solid->material)
    {
        _pdfprinterPrintMaterial(solid->material);
    }
    
    for (Polygon *poly = solid->first; NULL != poly; poly = (Polygon*)poly->next)
    {    
        if (poly->flags & GM_GOBJECT_FLAG_POLY_ABSOLUT)
        {
            _pdfprinterPrintPolygonAbsolut(poly, printer_drawing);
        }
        else
        {        
            _pdfprinterPrintPolygonRelative(poly, printer_drawing);
        }
    }
    
    return 0;
}    

const char* pdfprinterGetPDFNameFromModel(const char* fileName, char* buffer, int bufferSize)
{
    strncpy(buffer,fileName,bufferSize);
    
    char *configFileName = buffer;
    char *postFix = NULL;
    int restBufferSize = bufferSize;
    
    for (char *i = buffer;*i;i++)
    {
        if ('.' == *i)
        {
            *i = 0;
            postFix = i;
            break;
        }
        
        restBufferSize--;
    }
    
    if (postFix && (restBufferSize > (int)sizeof(PDF_EXTENSION)))
    {
        strcpy(postFix,PDF_EXTENSION);
    }
    else
    {
        strncpy(buffer,fileName,bufferSize);
        commenStringCat(buffer,PDF_EXTENSION,bufferSize);
    }
    
    return configFileName;
}

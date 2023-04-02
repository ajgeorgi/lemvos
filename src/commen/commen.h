#ifndef __COMMEN__
#define __COMMEN__

#include <time.h>
#include <stdio.h>

/* Commen for GV 12.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#define GM_APPLICATION "GM"
#define GM_VERSION "0.0.9"

#define GM_VERTEX_DIMENSION 3

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

#define GAPERAL_BACKGROUND  1
#define GAPERAL_FOREGROUND  2
#define GAPERAL_WIDGET      3
#define GAPERAL_WIDGET_DISABLE    4
#define GAPERAL_WINDOW_BACKGROUND    5
#define GAPERAL_WIDGET_HIGHLIGHT   6
#define GAPERAL_WIDGET_INVERSE_TEXT  7
#define GAPERAL_MODEL 8
#define GAPERAL_AXIS_X  9
#define GAPERAL_AXIS_Y  10
#define GAPERAL_AXIS_Z  11
#define GAPERAL_VERTEX_MARK  12
#define GAPERAL_POLY_START  13
#define GAPERAL_POLY_END  14
#define GAPERAL_MODEL_OWNER 15
#define GAPERAL_ERROR 16
#define GAPERAL_CHOICE_BUTTON_HIGHLIGHT 17
#define GAPERAL_VERTEX_SELECTED 18
#define GAPERAL_POLY_SELECTED 19
#define GAPERAL_SOLID_SELECTED 20
#define GAPERAL_NORMALE 21
#define GAPERAL_TRI 22
#define GAPERAL_INPUT_BACKGROUND 23
#define GAPERAL_INPUT_FRAME 24
#define GAPERAL_INPUT_FOREGROUND 25
#define GAPERAL_CURSOR 26
#define GAPERAL_INPUT_FOCUS 27
#define GAPERAL_POPUP_BACKGROUND 28
#define GAPERAL_FAILED 29
#define GAPERAL_BOUNDING_BOX 30
#define GAPERAL_GRID 31
#define GAPERAL_MESH 32
#define GAPERAL_MESH_SELECTED 33
#define GAPERAL_TEXT 34
#define GAPERAL_VERTEX_MARK2 35
#define GAPERAL_VERTEX_MARKERROR 36
#define GAPERAL_MEASUREMENT 37
#define GAPERAL_CENTER_POINT 38
#define GAPERAL_GRAVITY_MEA 39
#define GAPERAL_BOUYANCY_MEA 40
#define GAPERAL_OCEAN 41
#define GAPERAL_TEXT_HIGHLIGHT 42


#define GMATH_EARTH_RADIUS                 6371000.8   /* m */
#define GMATH_EARTH_RADIUS_EQUATOR         6378137.0   /* m */
#define GMATH_EARTH_RADIUS_MERIDIAN        6356752.314   /* m */
#define GMATH_EARTH_ECLIPTIC                    23.43747/* ° */
#define GMATH_EARTH_CIRCUMFERENCE            40075016.68 /* m */
#define GMATH_EARTH_GRAVITY_ACCELERATION     9.807 /* m/s² */
#define GMATH_PIXEL_PER_M                    1000.00L /* px / GMATH_PIXEL_PER_M = m */
#define GMATH_VAN_DER_WALLS_AIR_A            135.8 /* 10-³(J*m³)/mol² */
#define GMATH_VAN_DER_WALLS_AIR_B            36.4 /* 10-6*m³/mol */
#define GMATH_VAN_DER_WALLS_WATER_A          557.29 /* 10-³(J*m³)/mol² */
#define GMATH_VAN_DER_WALLS_WATER_B          31 /* 10-6*m³/mol */
#define GMATH_IDEAL_GAS_CONST_R              8.31447 /* J/mol/K */

#ifdef M_PIl
#define GMATH_PI  M_PIl
#else
#define GMATH_PI  M_PI
#endif


#define __STR(arg)  #arg
#define _STR(arg)   __STR(arg)


// This is only used for visualization
#define NORM_LENGTH 300.0

#define UNKNOWN_SYMBOL "<unknown>"
#define LEMVOS_PATH "LemvosPath"

/* Supposed to be always a 3 dimensional array */
typedef double EPoint;
typedef long double EPointL; 
typedef unsigned int magic_number;
typedef unsigned long RepresentationType;
typedef unsigned long color_type;


typedef struct tm LTime;

typedef void (*CommonErrorHandler)(int code, const char *text);


#define GM_TIME_FORMAT "%Z %a %e/%b/%Y %H:%M"
#define FILEMODTIME_CURRENT_FILENAME NULL
#define COMMEN_TYPE_SEPERATOR '.'
#define COMMEN_REPRESENTATION_SHIFT 14
#define COMMEN_REPRESENTATION_OBJECT_MASK ((1<<COMMEN_REPRESENTATION_SHIFT)-1)
#define COMMEN_REPRESENTATION_MODEL_MASK ~COMMEN_REPRESENTATION_OBJECT_MASK

// #define commenGetModelRepresentation(_o) ((_o)->rep_type & COMMEN_REPRESENTATION_MODEL_MASK)

#define MAX(_a,_b) ((_a) < (_b)?(_b):(_a))
#define MIN(_a,_b) ((_a) > (_b)?(_b):(_a))
#define ABS(_a) ((_a) < 0 ? -(_a) : (_a))
#define TORAD(_a) ((_a)*M_PI/180.0)
#define TODEG(_a) ((_a)*180.0/M_PI)
#define SQR(_x)   ((_x)*(_x))
#define SGN(_x) ((_x) < 0 ? -1 : 1)
#define SWAP(_n,_m) { unsigned _x = (_m); _m = _n; _n = _x; }


#ifdef _DEBUG

extern int _commenTerminate();
extern FILE* __err_log;

#define _LOG_FILE (__err_log?__err_log:stderr)

#define LOG_FLUSH fflush(_LOG_FILE)

#define LOG1(_format)  fprintf(_LOG_FILE,"%s:%i:" _format,__FILE__,__LINE__)
#define LOG(_format, ...)  fprintf(_LOG_FILE,"%s:%i:" _format,__FILE__,__LINE__, __VA_ARGS__)
#define ERROR1(_format)  fprintf(_LOG_FILE,"***:%s:%i:" _format,__FILE__,__LINE__)
#define ERROR(_format, ...)  fprintf(_LOG_FILE,"***:%s:%i:" _format,__FILE__,__LINE__, __VA_ARGS__)
#define FATAL1(_format, ...) (fprintf(_LOG_FILE,"******:%s:%i:" _format,__FILE__,__LINE__), _commenTerminate())
#define FATAL(_format, ...)  (fprintf(_LOG_FILE,"******:%s:%i:" _format,__FILE__,__LINE__, __VA_ARGS__), _commenTerminate())

#else

#define LOG1(_format)  fprintf(stderr,_format)
#define LOG(_format, ...)  fprintf(stderr,_format, __VA_ARGS__)
#define ERROR1(_format)  fprintf(stderr,"***:" _format)
#define ERROR(_format, ...)  fprintf(stderr,"***:" _format, __VA_ARGS__)
#define FATAL1(_format, ...)  { fprintf(stderr,"******:%s:%i:" _format,__FILE__,__LINE__); }
#define FATAL(_format, ...)  { fprintf(stderr,"******:%s:%i:" _format,__FILE__,__LINE__, __VA_ARGS__); }

#define LOG_FLUSH 

#endif


void initCommen(const char* appName, CommonErrorHandler errHandler);
void finalizeCommen();

void commenPrintError(int err, const char* text, ...);


void commenStringCat(char *dest, const char* src, int len);

// Don't pass malloced or strduped strings here.
// The pointer will be modified so you can't free() it anymore.
// Use static string arrays here or preserv the pointer for freeing
char* commenStripString(char *string);


int commenCopyString(char *str, const char* str1, size_t strsize);

int commenGetFileModTime(const char *filename, LTime *modification);

int commenTimeFromString(char *timestr, LTime *time);
EPoint* commenVertexFromString(char* param, EPoint *v, unsigned long long *flags);

double commenStringToDouble(const char* str);
long commenStringToLong(const char* str);
unsigned long commenStringToULong(const char* str);
long long commenStringToLongLong(const char* str);
unsigned long long commenStringToULongLong(const char* str);

const char *commenNameFromModel(const char* modelName);

const char *commenNextInStringList(const char* strlist, const char sep, char *buffer, int bufferSize);

const char *commenPrefixString(const char *prefix, char *string, int bufferSize);

int commenNow(LTime *time);
int commenIsTime(LTime *time);
long commenTimeDiff(LTime *time1, LTime *time2);

int commenIsLabel(const char* label);

int isFileExists(const char*fileName);
unsigned long commenFileSize(const char*fileName);

RepresentationType commenGetRepresentation(const char* name);

extern void *memory_aloc(size_t size);
extern void  memory_free(void *mem);
extern void *memory_realloc(void* mem, size_t size);
extern char *memory_strdup(const char* str);
extern char *memory_strndup(const char* str, size_t size);
extern int memory_is_my_memory(void *mem);


#ifdef _DEBUG_MEMORY
extern void memory_check();
#endif

#ifdef _DEBUG    
extern int _check_memory(const void *obj);
#else
#define _check_memory(_x) 1
#endif

#endif // __COMMEN__

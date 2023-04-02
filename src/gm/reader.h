#ifndef __READER__
#define __READER__

/* Reader for GV 13.10.2022


  Copyright: Andrej Georgi -> axgeorgi@gmail.com
*/

#include "gobject.h"
#include "vertex.h"

typedef char* (*TokenScanner)(void);
typedef int (*PushBackScanner)(const char* push);

typedef struct ReaderScannerT {
    TokenScanner scanner;
    PushBackScanner push_back;
    int file_type;
    int number_of_elements;
    int element_index;
    FILE *stream;
} ReaderScanner;

#define scnannerEOF(_s) ((NULL == (_s)->stream) || feof((_s)->stream))
#define scannerRead(_s,_buffer,_size) fread(_buffer,_size,1,(_s)->stream)
#define scannerSkip(_s,_bytes) fseek((_s)->stream,_bytes,SEEK_CUR)
#define scannerShutdown(_s) if ((_s)->stream) { fclose((_s)->stream); (_s)->stream = NULL; }


typedef const GObject* (*ReaderT)(ReaderScanner* scanner);

typedef const GObject* (*ReadObject)(IndexType index, ReaderScanner* scanner);


ReaderT initLemvosReader();


void resetReader(CommonErrorHandler error_handler);
int readerRegisterObject(ObjectType type, ReadObject reader);

const GObject *readerCreate(ReaderScanner* scanner);


char *readFromScanner(ReaderScanner *scanner);
int pushBackToScanner(ReaderScanner *scanner, const char* token);


#endif // __READER__
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
} ReaderScanner;


typedef const GObject* (*ReadObject)(IndexType index, ReaderScanner* scanner);

void initReader(CommonErrorHandler error_handler);
int readerRegisterObject(ObjectType type, ReadObject reader);

const GObject *readerCreate(ReaderScanner* scanner);

#endif // __READER__
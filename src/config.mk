# for compilation
#CC=clang
#CPP=clang++
CC=gcc
CPP=g++
OPT= -g

GNU_WARN= -Werror -Wall -Wextra -Winit-self -Wpacked
# -Wno-format-truncation -Wno-implicit-fallthrough
CLANG_WARN= -Werror -Wall -Wno-typedef-redefinition
WARN=$(GNU_WARN)

COMPILE= -std=gnu99
INCLUDE= -I../commen -I. -I/usr/local/include
# Try this -> -D_DEBUG_TODO -D_DEBUG_FIX -D_DEBUG_MEMORY
DEFS=-D_DEBUG -D_DEBUG_MEM -D_DEBUG_MEMORY

# for linking
LFLAGS= 
#LSTATIC=-static
#LSHARED=-shared
X11LIBSDIR=-L/usr/local/lib

# for testing
TEST=lemvos
TESTDIR= models/
TESTFILE = $(TEST).model

# in case mtrace() is used
MALLOC_TRACE=/tmp/memory.lemvos

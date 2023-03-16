# for compilation
OPT= -g
WARN= -Werror -Wall -Wextra -Winit-self -Wno-format-truncation -Wno-implicit-fallthrough
COMPILE= -std=gnu99
CPPCOMPILE=
INCLUDE= -I../commen -I.
# Try this -> -D_DEBUG_TODO -D_DEBUG_FIX
DEFS=-D_DEBUG -D_DEBUG_MEM

# for linking
LFLAGS= 
#LSTATIC=-static
#LSHARED=-shared

# for testing
TEST=lemvos
TESTDIR= models/
TESTFILE = $(TEST).model

# in case mtrace() is used
MALLOC_TRACE=/tmp/memory.lemvos

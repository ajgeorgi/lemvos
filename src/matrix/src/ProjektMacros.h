
//
// Some Macros for debugging purpose
// Written by Andrej Georgi (georgi@informatik.hu-berlin.de)
//
// The following defines decide which debug checks and output 
// are done
// CHECK     : When this variable is defined the check macros are enabled
//             otherwise check macros are disabled.
// WITH_INFO : Debug output is enabled. Otherwise debug output is disabled.
// USER_KEY  : This variable is defined to a string which represents the
//             key for debug output. When this variable is not defined
//             All debug output is enabled.
// WITH_DOGMON : Debug output for DogMon program.
//               

#ifndef _PROJEKTMACRO_H
#define _PROJEKTMACRO_H

#ifdef WITH_DOGMON
#ifdef USER_KEY
#undef USER_KEY
#endif
#ifndef WITH_INFO
#define WITH_INFO
#endif
#endif

#ifndef _BOOL_DEF
#define _BOOL_DEF
enum Bool { NO = 0, YES };
#endif

#include <iostream>

// Do we need the check at all
#ifdef CHECK

#ifndef __STRING
#define __STRING(__x) #__x
#endif

#define COUT cout
#define ENDL endl

// Do we need debug output
#ifdef WITH_INFO

extern Bool _keycmp(const char*,const char*);
extern const char *_theUserKey;

#ifdef WITH_DOGMON
#define INITINFO(_x) COUT << "<INIT>:"; _x; COUT << "</INIT>" << ENDL
#else // WITH_DOGMON
#define INITINFO(_x) _x
#endif // WITH_DOGMON

// Do we have a user key for debug output
#ifdef USER_KEY

#define INFO(_u,_x) if (_keycmp(USER_KEY,(_u)) == YES) { COUT << __FILE__ << ":" << __LINE__ << ":"; (_x);}

#define USER(_u,_x) if (_keycmp(USER_KEY,(_u)) == YES) { _x;}

#else // USER_KEY

#ifdef WITH_DOGMON
#define INFO(_u,_x) COUT << "<INFO>:" << __FILE__ << ":" << __LINE__ << ":" << "\"" << _u << "\":"; _x; COUT << "</INFO>"
#else // WITH_DOGMON
#define INFO(_u,_x) COUT << __FILE__ << ":" << __LINE__ << ":" << "\"" << _u << "\":"; _x
#endif // WITH_DOGMON

#define USER(_u,_x)  _x

#endif // USER_KEY

#else // WITH_INFO

#define INFO(_u,_x) /**/
#define USER(_u,_x) /**/
#define INITINFO(_x) /**/

#endif // WITH_INFO

#ifdef WITH_DOGMON

#define DOGTRACE(_u,_x,_y) cout << "<DOG>:\"" << _u << "\":" << _x << ":"<< _y << "</DOG>" << endl
#define BALLTRACE(_u,_x,_y) cout << "<BALL>:\"" << _u << "\":" << _x << ":" << _y << "</BALL>" << endl

#define VARTRACE(_u,_var) cout << "<VAR>:\"" << _u << "\":" << __FILE__ << ":" << __LINE__ << ":" << __STRING(_var) << "=" << _var << "</VAR>" << endl

#define OBJTRACE(_u,_x,_y) cout << "<OBJ>:\"" << _u << "\":" << _x << ":" << _y << "</OBJ>" << endl

#else // WITH_DOGMON

#define DOGTRACE(_u,_x,_y)  /**/
#define BALLTRACE(_u,_x,_y) /**/
#define VARTRACE(_u,_var) /**/
#define OBJTRACE(_u,_x,_y) /**/

#endif // WITH_DOGMON

#ifdef WITH_DOGMON
#define NOTIMP COUT << "<NOTIMP>:" << __FILE__ << ":" << __LINE__ << ":" << "Sorry. Not implemented function called." << ":</NOTIMP>" << ENDL
 
#define MISTAKE { char _c = 0; COUT << "<MISTAKE>:"__FILE__ << ":" << __LINE__ << ":" << "Fatal error. This code should never be reached!" << ":</MISTAKE>" << ENDL; cin >> _c; }

// Stops execution if expressen is not vallid
#define REQUIRE(_x) if (!(_x)) { char _c = 0; COUT << "<REQUIRE>:" << __FILE__ << ":" << __LINE__ << ": *** Requirement failed: " << __STRING(_x) << ":</REQUIRE>" << ENDL; cin.flush(); cin >> _c; }

#else  // WITH_DOGMON
#define NOTIMP COUT << __FILE__ << ":" << __LINE__ << ":" << "Sorry. Not implemented function called." << ENDL

#define MISTAKE { char _c = 0; COUT << __FILE__ << ":" << __LINE__ << ":" << "Fatal error. This code should never be reached!" << ENDL; cin >> _c; }

// Stops execution if expressen is not vallid
#define REQUIRE(_x) if (!(_x)) { char _c = 0; COUT << __FILE__ << ":" << __LINE__ << ": *** Requirement failed: " << __STRING(_x) << ENDL; cin >> _c; }

#endif // WITH_DOGMON

#else // CHECK

// No check. So remove all check macros.
#define INFO(_u,_x) /**/
#define USER(_u,_x) /**/
#define REQUIRE(_x) /**/
#define MISTAKE /**/
#define NOTIMP /**/
#define COUT cout
#define ENDL endl
#define INITINFO(_x) /**/
#define VARTRACE(_u,_v) /**/
#define DOGTRACE(_u,_x,_y) /**/
#define BALLTRACE(_u,_x,_y) /**/
#define OBJTRACE(_u,_x,_y) /**/

#endif // CHECK

#endif // _PROJEKTMACRO_H

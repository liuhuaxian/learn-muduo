// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

//__thread变量每一个线程有一份独立实体，各个线程的值互不干扰。
/*
 *
 char* abi::__cxa_demangle 	( 	const char *  	mangled_name,
		char *  	output_buffer,
		size_t *  	length,
		int *  	status	 
	) 			

New ABI-mandated entry point in the C++ runtime library for demangling.

Parameters:
    	mangled_name 	A NUL-terminated character string containing the name to be demangled.
    	output_buffer 	A region of memory, allocated with malloc, of *length bytes, into which the demangled name is stored. If output_buffer is not long enough, it is expanded using realloc. output_buffer may instead be NULL; in that case, the demangled name is placed in a region of memory allocated with malloc.
    	length 	If length is non-NULL, the length of the buffer containing the demangled name is placed in *length.
    	status 	*status is set to one of the following values:

        0: The demangling operation succeeded.
        -1: A memory allocation failiure occurred.
        -2: mangled_name is not a valid name under the C++ ABI mangling rules.
        -3: One of the arguments is invalid.

Returns:
    A pointer to the start of the NUL-terminated demangled name, or NULL if the demangling fails. The caller is responsible for deallocating this memory using free.
 *
*/

#include "muduo/base/CurrentThread.h"

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>

namespace muduo
{
namespace CurrentThread
{
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "unknown";
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

string stackTrace(bool demangle)
{
  string stack;
  const int max_frames = 200;
  void* frame[max_frames];
  int nptrs = ::backtrace(frame, max_frames);
  char** strings = ::backtrace_symbols(frame, nptrs);
  if (strings)
  {
    size_t len = 256;
    char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
    for (int i = 1; i < nptrs; ++i)  // skipping the 0-th, which is this function
    {
      if (demangle)
      {
        // https://panthema.net/2008/0901-stacktrace-demangled/
        // bin/exception_test(_ZN3Bar4testEv+0x79) [0x401909]
        char* left_par = nullptr;
        char* plus = nullptr;
        for (char* p = strings[i]; *p; ++p)
        {
          if (*p == '(')
            left_par = p;
          else if (*p == '+')
            plus = p;
        }

        if (left_par && plus)
        {
          *plus = '\0';
          int status = 0;
          //将c++标准的abi函数名，转换成代码中出现的函数名。
          char* ret = abi::__cxa_demangle(left_par+1, demangled, &len, &status);
          *plus = '+';
          if (status == 0)
          {
            demangled = ret;  // ret could be realloc()
            stack.append(strings[i], left_par+1);
            stack.append(demangled);
            stack.append(plus);
            stack.push_back('\n');
            continue;
          }
        }
      }
      // Fallback to mangled names
      stack.append(strings[i]);
      stack.push_back('\n');
    }
    free(demangled);
    free(strings);
  }
  return stack;
}

}  // namespace CurrentThread
}  // namespace muduo

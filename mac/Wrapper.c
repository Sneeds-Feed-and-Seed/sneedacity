/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Wrapper.c

  Sneedacity(R) is copyright (c) 2020-2021 Sneedacity Team.
  License: GPL v2.  See License.txt.

*******************************************************************//**

\file
Give the user more control over where libraries such as FFmpeg get
loaded from.

Since absolute pathnames are used when loading these libraries, the
normal search path would be DYLD_LIBRARY_PATH, absolute path,
DYLD_FALLBACK_LIBRARY_PATH.  This means that DYLD_LIBRARY_PATH can
override what the user actually wants.

So, we unset DYLD_LIBRARY_PATH variable and then invoke the main
executable.

*//*******************************************************************/

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SNEEDACITY_LEN 11
static const char sneedacity[] = "Sneedacity";
extern char **environ;

int main(int argc, char *argv[])
{
   size_t len = strnlen(argv[0], 256) + 1, // to account for terminating char
          exlen = len + SNEEDACITY_LEN;
          
   char *path = alloca(exlen);
   strncpy(path, argv[0], len);

   char *slash = strrchr(path, '/');
   if (slash)
   {
       strncpy(++slash, sneedacity, SNEEDACITY_LEN);
   }
   argv[0] = path; // argv[0] should now also be the Sneedacity path, not the Wrapper path

   unsetenv("DYLD_LIBRARY_PATH");

   execve(path, argv, environ);
}

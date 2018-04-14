/* File: utils.c */
/*
  This file is a part of the Corrfunc package
  Copyright (C) 2015-- Manodeep Sinha (manodeep@gmail.com)
  License: MIT LICENSE. See LICENSE file under the top-level
  directory at https://github.com/manodeep/Corrfunc/
*/

/*
  A collection of C wrappers I use. Should be
  very obvious. The ones that are not obvious
  have comments before the function itself.

  Bugs:
  Please email me manodeep at gmail dot com

  Ver 1.0: Manodeep Sinha, 2nd April, 2012
  Ver 1.1: Manodeep Sinha, 14th June, 2012 - replaced
  check_string_copy with a "real" wrapper to
  snprintf.
  Ver 1.2: Manodeep Sinha, Jan 8, 2012 - replaced
  print_time with timeval and gettimeofday
*/

#include <inttypes.h>    //defines PRId64 for printing int64_t + includes stdint.h
#include <math.h>
#include <string.h>
#include <limits.h>
#include <stdarg.h>

#include "macros.h"
#include "core_utils.h"

#ifdef __MACH__             // OS X does not have clock_gettime, use clock_get_time
#include <mach/mach_time.h> /* mach_absolute_time -> really fast */
#endif


// A real wrapper to snprintf that will exit() if the allocated buffer length
// was not sufficient. Usage is the same as snprintf
int my_snprintf(char *buffer, int len, const char *format, ...)
{
  va_list args;
  int nwritten = 0;

  va_start(args, format);
  nwritten = vsnprintf(buffer, (size_t)len, format, args);
  va_end(args);
  if (nwritten > len || nwritten < 0) {
    fprintf(stderr,
            "ERROR: printing to string failed (wrote %d characters while only %d characters were "
            "allocated)\n",
            nwritten, len);
    fprintf(stderr, "Increase `len'=%d in the header file\n", len);
    return -1;
  }
  return nwritten;
}

/*
  I like this particular function. Generic replacement for printing
  (in meaningful units) the actual execution time of a code/code segment.

  The function call should be like this:

  ---------------------------
  struct timeval t_start,t_end;
  gettimeofday(&t_start,NULL);
  do_something();
  gettimeofday(&t_end,NULL);
  print_time(t_start,t_end,"do something");
  ---------------------------

  if the code took 220 mins 30.1 secs
  -> print_time will output `Time taken to execute `do something' = 3 hours 40 mins 30.1 seconds


  (code can be easily extended to include `weeks' as a system of time unit. left to the reader)
*/

char *get_time_string(struct timeval t0, struct timeval t1)
{
  const size_t MAXLINESIZE = 1024;
  char *time_string = malloc(MAXLINESIZE * sizeof(char));
  if(time_string == NULL)  {
    fprintf(stderr,"Error: Could not allocate memory to hold string variable representing time in function '%s'..returning\n", __FUNCTION__);
    return NULL;
  }
  double timediff = t1.tv_sec - t0.tv_sec;
  double ratios[] = {24 * 3600.0, 3600.0, 60.0, 1};
  char units[4][10] = {"days", "hrs", "mins", "secs"};
  int which = 0;

  double timeleft = timediff;
  double time_to_print;

  if (timediff < ratios[2]) {
    my_snprintf(time_string, MAXLINESIZE, "%6.3lf secs",
                1e-6 * (t1.tv_usec - t0.tv_usec) + timediff);
  } else {
    size_t curr_index = 0;
    while (which < 4) {
      time_to_print = floor(timeleft / ratios[which]);
      if (time_to_print > 1) {
        timeleft -= (time_to_print * ratios[which]);
        char tmp[MAXLINESIZE];
        my_snprintf(tmp, MAXLINESIZE, "%5d %s", (int)time_to_print, units[which]);
        const size_t len = strlen(tmp);
        const size_t required_len = curr_index + len + 1;
        XRETURN(MAXLINESIZE >= required_len, NULL,
                "buffer overflow will occur: string has space for %zu bytes while concatenating "
                "requires at least %zu bytes\n",
                MAXLINESIZE, required_len);
        strcpy(time_string + curr_index, tmp);
        curr_index += len;
      }
      which++;
    }
  }

  return time_string;
}



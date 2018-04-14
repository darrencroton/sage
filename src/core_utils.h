/* File: utils.h */
/*
  This file is a part of the Corrfunc package
  Copyright (C) 2015-- Manodeep Sinha (manodeep@gmail.com)
  License: MIT LICENSE. See LICENSE file under the top-level
  directory at https://github.com/manodeep/Corrfunc/
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>    //defines int64_t datatype -> *exactly* 8 bytes int
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0    
/* To prevent text editors from being confused about the open brace*/
}
#endif
    
extern int my_snprintf(char *buffer, int len, const char *format, ...)
    __attribute__((format(printf, 3, 4)));
extern char *get_time_string(struct timeval t0, struct timeval t1);



#ifdef __cplusplus
}
#endif



    

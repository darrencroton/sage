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

    extern int my_snprintf(char *buffer, int len, const char *format, ...)
        __attribute__((format(printf, 3, 4)));
    extern char *get_time_string(struct timeval t0, struct timeval t1);
    extern size_t myfread(void *ptr, const size_t size, const size_t nmemb, FILE * stream);
    extern size_t myfwrite(const void *ptr, const size_t size, const size_t nmemb, FILE * stream);
    extern int myfseek(FILE * stream, const long offset, const int whence);
    extern ssize_t mypread(int fd, void *ptr, const size_t nbytes, off_t offset);
    extern ssize_t mypwrite(int fd, const void *ptr, const size_t nbytes, off_t offset);
    
#ifdef __cplusplus
}
#endif



    

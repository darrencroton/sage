#pragma once

#ifdef __cplusplus
extern "C" {
#endif

    /* functions in core_cool_func.c */
    extern void read_cooling_functions(void);
    extern double get_metaldependent_cooling_rate(const double logTemp, double logZ);

#ifdef __cplusplus
}
#endif

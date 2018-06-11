#pragma once

#include "core_allvars.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0    
/* To prevent text editors from being confused about the open brace*/
}
#endif

void init_sage(const int ThisTask, const char *param_file);
void sage(const int ThisTask, const int NTasks);
void finalize_sage(void);

#ifdef __cplusplus
}
#endif



    

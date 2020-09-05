#ifndef __RK_AIQ_TYPES_AFEC_ALGO_INT_H__
#define __RK_AIQ_TYPES_AFEC_ALGO_INT_H__

#include "rk_aiq_types_afec_algo.h"

typedef struct rk_aiq_fec_cfg_s {
    unsigned int en;
    int bypass;
    unsigned char correct_level;
} rk_aiq_fec_cfg_t;

#endif

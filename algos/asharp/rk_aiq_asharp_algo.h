
#ifndef __RKAIQ_ASHARP_H__
#define __RKAIQ_ASHARP_H__

#include "stdio.h"
#include "string.h"
#include "math.h"
#include "rk_aiq_asharp_algo_sharp.h"
#include "rk_aiq_asharp_algo_edgefilter.h"
#include "rk_aiq_types_asharp_algo_prvt.h"


RKAIQ_BEGIN_DECLARE

AsharpResult_t AsharpInit(AsharpContext_t **ppAsharpCtx, CamCalibDbContext_t *pCalibDb);

AsharpResult_t AsharpRelease(AsharpContext_t *pAsharpCtx);

AsharpResult_t AsharpPrepare(AsharpContext_t *pAsharpCtx, AsharpConfig_t* pAsharpConfig);

AsharpResult_t AsharpReConfig(AsharpContext_t *pAsharpCtx, AsharpConfig_t* pAsharpConfig);

AsharpResult_t AsharpPreProcess(AsharpContext_t *pAsharpCtx);

AsharpResult_t AsharpProcess(AsharpContext_t *pAsharpCtx, int ISO);

AsharpResult_t AsharpGetProcResult(AsharpContext_t *pAsharpCtx, AsharpProcResult_t* pAsharpResult);

AsharpResult_t Asharp_fix_Printf(RKAsharp_Sharp_HW_Fix_t  * pSharpCfg, RKAsharp_Edgefilter_Fix_t *pEdgefltCfg);

RKAIQ_END_DECLARE

#endif

#include "rk_aiq_types.h"

int32_t ae_init(int32_t s32Handle, const rk_aiq_ae_param_t *pstAeParam);

int32_t ae_run(int32_t s32Handle, const rk_aiq_ae_statistics_t *pstAeInfo,
                     rk_aiq_ae_result_t *pstAeResult, int32_t s32Rsv);

int32_t ae_ctrl(int32_t s32Handle, uint32_t u32Cmd, void *pValue);

int32_t ae_exit(int32_t s32Handle);

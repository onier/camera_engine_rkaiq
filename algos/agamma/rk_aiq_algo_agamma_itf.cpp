/*
 * rk_aiq_algo_agamma_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "rk_aiq_algo_types_int.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "agamma/rk_aiq_agamma_algo.h"
RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    void* place_holder[0];
} RkAiqAlgoContext;



static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t*AgammaHandle = NULL;
    AlgoCtxInstanceCfgInt* instanc_int = (AlgoCtxInstanceCfgInt*)cfg;
    CamCalibDbContext_t* calib = instanc_int->calib;
    ret = AgammaInit(&AgammaHandle, calib);
    *context = (RkAiqAlgoContext *)(AgammaHandle);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    AgammaHandle_t*AgammaHandle = (AgammaHandle_t*)context;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AgammaRelease(AgammaHandle);

    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * AgammaHandle = (AgammaHandle_t *)params->ctx;
    RkAiqAlgoConfigAgammaInt* pCfgParam = (RkAiqAlgoConfigAgammaInt*)params;
    rk_aiq_gamma_cfg_t *agamma_config = &AgammaHandle->agamma_config;

    if (RK_AIQ_WORKING_MODE_NORMAL == pCfgParam->agamma_config_com.com.u.prepare.working_mode){
        AgammaHandle->last_mode = agamma_config->gamma_out_mode = GAMMA_OUT_NORMAL;
        memcpy(agamma_config->gamma_table, AgammaHandle->normal_table, sizeof(AgammaHandle->normal_table));
    }else{
        AgammaHandle->last_mode = agamma_config->gamma_out_mode = GAMMA_OUT_HDR;
        memcpy(agamma_config->gamma_table, AgammaHandle->hdr_table, sizeof(AgammaHandle->hdr_table));
    }
    return ret;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoPreAgammaInt* pAgammaPreParams = (RkAiqAlgoPreAgammaInt*)inparams;
    AgammaHandle_t * AgammaHandle = (AgammaHandle_t *)inparams->ctx;
    rk_aiq_gamma_cfg_t *agamma_config = &AgammaHandle->agamma_config;

    if (pAgammaPreParams->rk_com.u.proc.gray_mode) {
        AgammaHandle->last_mode = agamma_config->gamma_out_mode;
        agamma_config->gamma_out_mode = GAMMA_OUT_NIGHT;
        memcpy(agamma_config->gamma_table, AgammaHandle->night_table, sizeof(AgammaHandle->night_table));
    }else if (GAMMA_OUT_NORMAL == AgammaHandle->last_mode){
        agamma_config->gamma_out_mode = GAMMA_OUT_NORMAL;
        memcpy(agamma_config->gamma_table, AgammaHandle->normal_table, sizeof(AgammaHandle->normal_table));
    }else{
        agamma_config->gamma_out_mode = GAMMA_OUT_HDR;
        memcpy(agamma_config->gamma_table, AgammaHandle->hdr_table, sizeof(AgammaHandle->hdr_table));
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * AgammaHandle = (AgammaHandle_t *)inparams->ctx;
    RkAiqAlgoProcResAgamma* procResPara = (RkAiqAlgoProcResAgamma*)outparams;
    rk_aiq_gamma_cfg_t* agamma_config = (rk_aiq_gamma_cfg_t*)&procResPara->agamma_config;
    memcpy(agamma_config, &AgammaHandle->agamma_config, sizeof(rk_aiq_gamma_cfg_t));
    return ret;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgamma = {
    .common = {
        .version = RKISP_ALGO_AGAMMA_VERSION,
        .vendor  = RKISP_ALGO_AGAMMA_VENDOR,
        .description = RKISP_ALGO_AGAMMA_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAMMA,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE

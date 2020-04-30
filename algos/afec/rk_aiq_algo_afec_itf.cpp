/*
 * rk_aiq_algo_afec_itf.c
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
#include "afec/rk_aiq_algo_afec_itf.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

typedef struct FECContext_s {
    unsigned char initialized;
    unsigned int fec_en;
    unsigned int mesh_density; //0:16x8 1:32x16
    unsigned int fec_mesh_h_size;
    unsigned int fec_mesh_v_size;
    unsigned int fec_mesh_size;
    unsigned int pic_width;
    unsigned int pic_height;
    unsigned int sw_rd_vir_stride;
    unsigned int sw_wr_yuv_format; //0:YUV420 1:YUV422
    unsigned int sw_wr_vir_stride;
    unsigned int sw_fec_wr_fbce_mode; //0:normal 1:fbec
    unsigned short* meshxi;
    unsigned char* meshxf;
    unsigned short* meshyi;
    unsigned char* meshyf;
} FECContext_t;

typedef struct FECContext_s* FECHandle_t;
static FECContext_t gFECCtx;

typedef struct _RkAiqAlgoContext {
    FECHandle_t hFEC;
    void* place_holder[0];
} RkAiqAlgoContext;

static RkAiqAlgoContext ctx;

static XCamReturn
gen_mesh_table(
        float k1, float k2, float p1, float p2, float k3,
        int pic_width, int pic_height,
        int mesh_h_size, int mesh_v_size,
        int* meshxi, int* meshxf, int* meshyi, int* meshyf) {
    //TODO::implement mesh table generation function
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    memset(&gFECCtx, 0, sizeof(FECContext_t));
    ctx.hFEC = &gFECCtx;
    *context = &ctx;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    FECHandle_t hFEC = (FECHandle_t)context->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;

    if (fecCtx->meshxi != NULL) {
        free(fecCtx->meshxi);
        fecCtx->meshxi = NULL;
    }

    if (fecCtx->meshyi != NULL) {
        free(fecCtx->meshyi);
        fecCtx->meshyi = NULL;
    }

    if (fecCtx->meshxf != NULL) {
        free(fecCtx->meshxf);
        fecCtx->meshxf = NULL;
    }

    if (fecCtx->meshyf != NULL) {
        free(fecCtx->meshyf);
        fecCtx->meshyf = NULL;
    }
    return XCAM_RETURN_NO_ERROR;
}

#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define ALIGN(x,a)		__ALIGN_MASK(x, a-1)

uint32_t cal_fec_mesh(uint32_t width, uint32_t height, uint32_t mode, uint32_t &meshw, uint32_t &meshh)
{
    uint32_t mesh_size, mesh_left_height;
    uint32_t w = ALIGN(width, 32);
    uint32_t h = ALIGN(height, 32);
    uint32_t spb_num = (h + 127) >> 7;
    uint32_t left_height = h & 127;
    uint32_t mesh_width = mode ? (w / 32 + 1) : (w / 16 + 1);
    uint32_t mesh_height = mode ? 9 : 17;

    if (!left_height)
        left_height = 128;
    mesh_left_height = mode ? (left_height / 16 + 1) :
                (left_height / 8 + 1);
    mesh_size = (spb_num - 1) * mesh_width * mesh_height +
        mesh_width * mesh_left_height;

    meshw = mesh_width;
    meshh = (spb_num - 1) * mesh_height + (spb_num - 1);
    return mesh_size;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    FECHandle_t hFEC = (FECHandle_t)params->ctx->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
    RkAiqAlgoConfigAfecInt* rkaiqAfecConfig = (RkAiqAlgoConfigAfecInt*)params;

    fecCtx->fec_en = rkaiqAfecConfig->afec_calib_cfg.fec_en;

    fecCtx->pic_width = params->u.prepare.sns_op_width;
    fecCtx->pic_height = params->u.prepare.sns_op_height;

    if (fecCtx->pic_width <= 1920) {
        fecCtx->mesh_density = 0;
    } else {
        fecCtx->mesh_density = 1;
    }

    fecCtx->fec_mesh_size =
        cal_fec_mesh(fecCtx->pic_width, fecCtx->pic_height, fecCtx->mesh_density,
            fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size);

    LOGI_AFEC("(%s) en: %d, dimen: %d-%d, mesh dimen: %d-%d, size: %d",
        rkaiqAfecConfig->afec_calib_cfg.meshfile, fecCtx->fec_en,
        fecCtx->pic_width, fecCtx->pic_height,
        fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size,
        fecCtx->fec_mesh_size);

    fecCtx->meshxi = (unsigned short*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned short));
    fecCtx->meshxf = (unsigned char*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned char));
    fecCtx->meshyi = (unsigned short*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned short));
    fecCtx->meshyf = (unsigned char*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned char));

#if OPENCV_SUPPORT
    gen_default_mesh_table(fecCtx->pic_width, fecCtx->pic_height, fecCtx->mesh_density,
        fecCtx->meshxi, fecCtx->meshyi, fecCtx->meshxf, fecCtx->meshyf);
#else
    FILE* ofp;
    char filename[512];
    sprintf(filename, "/etc/iqfiles/%s/meshxi.bin", rkaiqAfecConfig->afec_calib_cfg.meshfile);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshxi, 1, fecCtx->fec_mesh_size * sizeof(unsigned short), ofp);
        fclose(ofp);

        if (num != fecCtx->fec_mesh_size * sizeof(unsigned short)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh XI file");
        }
    } else {
        LOGW_AFEC("mesh XI file not exist");
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "/etc/iqfiles/%s/meshxf.bin", rkaiqAfecConfig->afec_calib_cfg.meshfile);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshxf, 1, fecCtx->fec_mesh_size * sizeof(unsigned char), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned char)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh XF file");
        }
    } else {
        LOGW_AFEC("mesh XF file not exist");
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "/etc/iqfiles/%s/meshyi.bin", rkaiqAfecConfig->afec_calib_cfg.meshfile);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshyi, 1, fecCtx->fec_mesh_size * sizeof(unsigned short), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned short)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh YI file");
        }
    } else {
        LOGW_AFEC("mesh YI file not exist");
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "/etc/iqfiles/%s/meshyf.bin", rkaiqAfecConfig->afec_calib_cfg.meshfile);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshyf, 1, fecCtx->fec_mesh_size * sizeof(unsigned char), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned char)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh YF file");
        }
    } else {
        LOGW_AFEC("mesh YF file not exist");
        fecCtx->fec_en = 0;
    }
#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    FECHandle_t hFEC = (FECHandle_t)inparams->ctx->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
    RkAiqAlgoProcAfecInt* fecProcParams = (RkAiqAlgoProcAfecInt*)inparams;

    RkAiqAlgoProcResAfecInt* fecPreOut = (RkAiqAlgoProcResAfecInt*)outparams;
    fecPreOut->afec_result.sw_fec_en = fecCtx->fec_en;
    fecPreOut->afec_result.crop_en = 0;
    fecPreOut->afec_result.crop_width = 0;
    fecPreOut->afec_result.crop_height = 0;
    fecPreOut->afec_result.mesh_density = fecCtx->mesh_density;
    fecPreOut->afec_result.mesh_size = fecCtx->fec_mesh_size;
    memcpy(fecPreOut->afec_result.meshxi, fecCtx->meshxi,
        fecCtx->fec_mesh_size*sizeof(unsigned short));
    memcpy(fecPreOut->afec_result.meshxf, fecCtx->meshxf,
        fecCtx->fec_mesh_size*sizeof(unsigned char));
    memcpy(fecPreOut->afec_result.meshyi, fecCtx->meshyi,
        fecCtx->fec_mesh_size*sizeof(unsigned short));
    memcpy(fecPreOut->afec_result.meshyf, fecCtx->meshyf,
        fecCtx->fec_mesh_size*sizeof(unsigned char));

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAfec = {
    .common = {
        .version = RKISP_ALGO_AFEC_VERSION,
        .vendor  = RKISP_ALGO_AFEC_VENDOR,
        .description = RKISP_ALGO_AFEC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AFEC,
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

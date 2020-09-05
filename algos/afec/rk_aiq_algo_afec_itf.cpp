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
#include "afec/rk_aiq_types_afec_algo_prvt.h"
#include "xcam_log.h"

#define EPSINON 0.0000001

RKAIQ_BEGIN_DECLARE

static XCamReturn
gen_mesh_table(
    float k1, float k2, float p1, float p2, float k3,
    int src_width, int src_height,
    int mesh_h_size, int mesh_v_size,
    int* meshxi, int* meshxf, int* meshyi, int* meshyf) {
    //TODO::implement mesh table generation function
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AFEC( "%s: create afec context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->hFEC = new FECContext_t;
    if (ctx->hFEC == NULL) {
        LOGE_AFEC( "%s: create afec handle fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset((void *)(ctx->hFEC), 0, sizeof(FECContext_t));
    *context = ctx;
#if GENMESH_ONLINE
    ctx->hFEC->afecReadMeshThread = new RKAiqAfecThread(ctx->hFEC);
    ctx->hFEC->afecReadMeshThread->triger_start();
    ctx->hFEC->afecReadMeshThread->start();
#endif
    ctx->hFEC->eState = FEC_STATE_INVALID;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    FECHandle_t hFEC = (FECHandle_t)context->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
#if GENMESH_ONLINE
    fecCtx->afecReadMeshThread->triger_stop();
    fecCtx->afecReadMeshThread->stop();
    if (fecCtx->meshxi != NULL || fecCtx->meshyi != NULL || \
        fecCtx->meshxf != NULL || fecCtx->meshyf != NULL)
        freeFecMesh(fecCtx->meshxi, fecCtx->meshxf,
                fecCtx->meshyi, fecCtx->meshyf);
    genFecMeshDeInit(fecCtx->fecParams);
#else
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
#endif
    delete context->hFEC;
    context->hFEC = NULL;
    delete context;
    context = NULL;
    return XCAM_RETURN_NO_ERROR;
}

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)      __ALIGN_MASK(x, a-1)

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
read_mesh_table(FECContext_t* fecCtx, unsigned int correct_level)
{
    bool ret;
#if OPENCV_SUPPORT
    gen_default_mesh_table(fecCtx->src_width, fecCtx->src_height, fecCtx->mesh_density,
                           fecCtx->meshxi, fecCtx->meshyi, fecCtx->meshxf, fecCtx->meshyf);
#elif GENMESH_ONLINE
    ret = genFECMeshNLevel(fecCtx->fecParams, fecCtx->camCoeff, correct_level, fecCtx->meshxi,
                           fecCtx->meshxf, fecCtx->meshyi, fecCtx->meshyf);
    if (!ret) {
        LOGE_AFEC("afec gen mesh false!");
        return XCAM_RETURN_ERROR_FAILED;
    }
#else
    FILE* ofp;
    char filename[512];
    sprintf(filename, "%s/%s/meshxi_level%d.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshxi, 1, fecCtx->fec_mesh_size * sizeof(unsigned short), ofp);
        fclose(ofp);

        if (num != fecCtx->fec_mesh_size * sizeof(unsigned short)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh XI file");
        }
    } else {
        LOGE_AFEC("mesh XI file %s not exist", filename);
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "%s/%s/meshxf_level%d.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshxf, 1, fecCtx->fec_mesh_size * sizeof(unsigned char), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned char)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh XF file");
        }
    } else {
        LOGE_AFEC("mesh XF file %s not exist", filename);
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "%s/%s/meshyi_level%d.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshyi, 1, fecCtx->fec_mesh_size * sizeof(unsigned short), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned short)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh YI file");
        }
    } else {
        LOGE_AFEC("mesh YI file %s not exist", filename);
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "%s/%s/meshyf_level%d.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshyf, 1, fecCtx->fec_mesh_size * sizeof(unsigned char), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned char)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh YF file");
        }
    } else {
        LOGE_AFEC("mesh YF file %s not exist", filename);
        fecCtx->fec_en = 0;
    }
#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    FECHandle_t hFEC = (FECHandle_t)params->ctx->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
    RkAiqAlgoConfigAfecInt* rkaiqAfecConfig = (RkAiqAlgoConfigAfecInt*)params;

    fecCtx->fec_en = rkaiqAfecConfig->afec_calib_cfg.fec_en;
    memcpy(fecCtx->meshfile, rkaiqAfecConfig->afec_calib_cfg.meshfile, sizeof(fecCtx->meshfile));
    fecCtx->camCoeff.cx = rkaiqAfecConfig->afec_calib_cfg.light_center[0];
    fecCtx->camCoeff.cy = rkaiqAfecConfig->afec_calib_cfg.light_center[1];
    fecCtx->camCoeff.a0 = rkaiqAfecConfig->afec_calib_cfg.coefficient[0];
    fecCtx->camCoeff.a2 = rkaiqAfecConfig->afec_calib_cfg.coefficient[1];
    fecCtx->camCoeff.a3 = rkaiqAfecConfig->afec_calib_cfg.coefficient[2];
    fecCtx->camCoeff.a4 = rkaiqAfecConfig->afec_calib_cfg.coefficient[3];
    LOGI_AFEC("(%s) len light center(%.16f, %.16f)\n",
            __FUNCTION__,
            fecCtx->camCoeff.cx, fecCtx->camCoeff.cy);
    LOGI_AFEC("(%s) len coefficient(%.16f, %.16f, %.16f, %.16f)\n",
            __FUNCTION__,
            fecCtx->camCoeff.a0, fecCtx->camCoeff.a2,
            fecCtx->camCoeff.a3, fecCtx->camCoeff.a4);

    fecCtx->src_width = params->u.prepare.sns_op_width;
    fecCtx->src_height = params->u.prepare.sns_op_height;
    fecCtx->resource_path = rkaiqAfecConfig->resource_path;
    fecCtx->dst_width = params->u.prepare.sns_op_width;
    fecCtx->dst_height = params->u.prepare.sns_op_height;

    if (fecCtx->src_width <= 1920) {
        fecCtx->mesh_density = 0;
    } else {
        fecCtx->mesh_density = 1;
    }

    double correct_level = rkaiqAfecConfig->afec_calib_cfg.correct_level;
    if (fecCtx->isAttribUpdated) {
        if (fecCtx->user_config.bypass)
            correct_level = 0;
        fecCtx->fec_en = fecCtx->user_config.en;
        fecCtx->isAttribUpdated = false;
    } else {
        fecCtx->user_config.en = fecCtx->fec_en;
    }
    fecCtx->user_config.correct_level = correct_level ;

#if GENMESH_ONLINE
    fecCtx->user_config.correct_level = correct_level;
    genFecMeshInit(fecCtx->src_width, fecCtx->src_height, fecCtx->dst_width,
            fecCtx->dst_height, fecCtx->fecParams, fecCtx->camCoeff);
    mallocFecMesh(fecCtx->fecParams.meshSize4bin, &fecCtx->meshxi,
            &fecCtx->meshxf, &fecCtx->meshyi, &fecCtx->meshyf);
    fecCtx->fec_mesh_size = fecCtx->fecParams.meshSize4bin;
    LOGI_AFEC("(%s) en: %d, bypass(%d), correct_level(%d), dimen(%d-%d), mesh dimen(%d-%d), size(%d)",
              rkaiqAfecConfig->afec_calib_cfg.meshfile, fecCtx->fec_en,
              fecCtx->user_config.bypass, fecCtx->user_config.correct_level,
              fecCtx->src_width, fecCtx->src_height,
              fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size,
              fecCtx->fec_mesh_size);
#else
    if (fabs(correct_level) <= fabs(EPSINON)) {
        fecCtx->user_config.correct_level = FEC_BYPASS;
    } else if (correct_level >= 1.0) {
        fecCtx->user_config.correct_level = FEC_CORRECT_LEVEL0;
    } else if (1 - correct_level <= 0.25) {
        fecCtx->user_config.correct_level = FEC_CORRECT_LEVEL1;
    } else if (1 - correct_level <= 0.50) {
        fecCtx->user_config.correct_level = FEC_CORRECT_LEVEL2;
    } else if (1 - correct_level <= 0.75) {
        fecCtx->user_config.correct_level = FEC_CORRECT_LEVEL3;
    } else {
        fecCtx->user_config.correct_level = FEC_BYPASS;
    }

    fecCtx->fec_mesh_size =
        cal_fec_mesh(fecCtx->src_width, fecCtx->src_height, fecCtx->mesh_density,
                     fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size);

    LOGI_AFEC("(%s) en: %d, user_en: %d, correct_level: %d, dimen: %d-%d, mesh dimen: %d-%d, size: %d",
              rkaiqAfecConfig->afec_calib_cfg.meshfile, fecCtx->fec_en,
              fecCtx->user_config.en, fecCtx->user_config.correct_level,
              fecCtx->src_width, fecCtx->src_height,
              fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size,
              fecCtx->fec_mesh_size);
    if (!fecCtx->fec_en)
        return XCAM_RETURN_NO_ERROR;
    // need realloc ?
    if (fecCtx->meshxi) {
        free(fecCtx->meshxi);
        fecCtx->meshxi = NULL;
    }
    if (fecCtx->meshxf) {
        free(fecCtx->meshxf);
        fecCtx->meshxf = NULL;
    }
    if (fecCtx->meshyi) {
        free(fecCtx->meshyi);
        fecCtx->meshyi = NULL;
    }
    if (fecCtx->meshyf) {
        free(fecCtx->meshyf);
        fecCtx->meshyf = NULL;
    }
    fecCtx->meshxi = (unsigned short*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned short));
    fecCtx->meshxf = (unsigned char*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned char));
    fecCtx->meshyi = (unsigned short*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned short));
    fecCtx->meshyf = (unsigned char*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned char));
#endif

    read_mesh_table(fecCtx, fecCtx->user_config.correct_level);
    fecCtx->eState = FEC_STATE_INITIALIZED;

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
    // TODO: should check the fec mode,
    // if mode == RK_AIQ_ISPP_STATIC_FEC_WORKING_MODE_STABLIZATION
    // params may be changed
    if (!fecCtx->fec_en)
        return XCAM_RETURN_NO_ERROR;
    fecCtx->eState = FEC_STATE_RUNNING;

    if (inparams->u.proc.init) {
        fecPreOut->afec_result.update = 1;
    } else {

        if (fecCtx->isAttribUpdated) {
            fecCtx->isAttribUpdated = false;
            fecPreOut->afec_result.update = 1;
        } else {
            fecPreOut->afec_result.update = 0;
        }

        LOGV_AFEC("en(%d), user en(%d), bypass(%d), level(%d), result update(%d)\n",
                fecCtx->fec_en,
                fecCtx->user_config.en,
                fecCtx->user_config.bypass,
                fecCtx->user_config.correct_level,
                fecPreOut->afec_result.update);
    }

    if (fecPreOut->afec_result.update) {
        memcpy(fecPreOut->afec_result.meshxi, fecCtx->meshxi,
               fecCtx->fec_mesh_size * sizeof(unsigned short));
        memcpy(fecPreOut->afec_result.meshxf, fecCtx->meshxf,
               fecCtx->fec_mesh_size * sizeof(unsigned char));
        memcpy(fecPreOut->afec_result.meshyi, fecCtx->meshyi,
               fecCtx->fec_mesh_size * sizeof(unsigned short));
        memcpy(fecPreOut->afec_result.meshyf, fecCtx->meshyf,
               fecCtx->fec_mesh_size * sizeof(unsigned char));
    }

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

bool RKAiqAfecThread::loop()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<rk_aiq_fec_cfg_t> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGW_ANALYZER("RKAiqAfecThread got empty attrib, stop thread");
        return false;
    }

    if (hFEC->eState != FEC_STATE_RUNNING) {
        hFEC->isAttribUpdated = true;
        return true;
    }

    if (hFEC->user_config.bypass) {
        ret = read_mesh_table(hFEC, 0);
    } else {
        ret = read_mesh_table(hFEC, attrib->correct_level);
    }

    if (ret == XCAM_RETURN_NO_ERROR) {
        hFEC->isAttribUpdated = true;
        return true;
    }

    LOGE_ANALYZER("RKAiqAfecThread failed to read mesh table!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

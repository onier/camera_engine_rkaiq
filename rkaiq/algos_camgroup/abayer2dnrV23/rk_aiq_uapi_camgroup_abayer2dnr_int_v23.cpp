#include "abayer2dnrV23/rk_aiq_uapi_camgroup_abayer2dnr_int_v23.h"
#include "rk_aiq_types_camgroup_abayer2dnr_prvt_v23.h"


#if 1
#define RAWNR_LUMA_SF_STRENGTH_SLOPE_FACTOR (8.0)


XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV23_SetAttrib(RkAiqAlgoContext *ctx,
        const rk_aiq_bayer2dnr_attrib_v23_t *attr,
        bool need_sync)
{
    CamGroup_Abayer2dnrV23_Contex_t *pGroupCtx = (CamGroup_Abayer2dnrV23_Contex_t *)ctx;
    Abayer2dnr_Context_V23_t* pCtx = pGroupCtx->abayer2dnr_contex_v23;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == ABAYER2DNR_V23_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == ABAYER2DNR_V23_OP_MODE_MANUAL) {
        pCtx->stManual.st2DSelect = attr->stManual.st2DSelect;
    } else if(pCtx->eMode == ABAYER2DNR_V23_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st2Dfix = attr->stManual.st2Dfix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV23_GetAttrib(const RkAiqAlgoContext *ctx,
        rk_aiq_bayer2dnr_attrib_v23_t *attr)
{

    CamGroup_Abayer2dnrV23_Contex_t *pGroupCtx = (CamGroup_Abayer2dnrV23_Contex_t *)ctx;
    Abayer2dnr_Context_V23_t* pCtx = pGroupCtx->abayer2dnr_contex_v23;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV23_SetStrength(const RkAiqAlgoContext *ctx,
        const rk_aiq_bayer2dnr_strength_v23_t* pStrength)
{
    CamGroup_Abayer2dnrV23_Contex_t *pGroupCtx = (CamGroup_Abayer2dnrV23_Contex_t *)ctx;
    Abayer2dnr_Context_V23_t* pCtx = (Abayer2dnr_Context_V23_t*)ctx;

    float fStrength = 1.0f;
    float fslope = RAWNR_LUMA_SF_STRENGTH_SLOPE_FACTOR;
    float fPercent = 0.5f;

    fPercent = pStrength->percent;


    if(fPercent <= 0.5) {
        fStrength = fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 * fslope / (1.0 - fPercent) - fslope + 1;
    }

    pCtx->stStrength = *pStrength;
    pCtx->stStrength.percent = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}




XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV23_GetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_bayer2dnr_strength_v23_t* pStrength)
{
    CamGroup_Abayer2dnrV23_Contex_t *pGroupCtx = (CamGroup_Abayer2dnrV23_Contex_t *)ctx;
    Abayer2dnr_Context_V23_t* pCtx = (Abayer2dnr_Context_V23_t*)ctx;

    float fStrength = 1.0f;
    float fslope = RAWNR_LUMA_SF_STRENGTH_SLOPE_FACTOR;
    float fPercent = 0.0f;

    fStrength = pCtx->stStrength.percent;

    if(fStrength <= 1) {
        fPercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp = 1 - 0.5 * fslope / (fStrength + fslope - 1);
        if(abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        fPercent = tmp;
    }

    *pStrength = pCtx->stStrength;
    pStrength->percent = fPercent;

    return XCAM_RETURN_NO_ERROR;
}



#endif


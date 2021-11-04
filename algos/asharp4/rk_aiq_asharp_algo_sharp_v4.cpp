
#include "rk_aiq_asharp_algo_sharp_v4.h"

RKAIQ_BEGIN_DECLARE

Asharp4_result_t sharp_select_params_by_ISO_V4(
    RK_SHARP_Params_V4_t *pParams,
    RK_SHARP_Params_V4_Select_t *pSelect,
    Asharp4_ExpInfo_t *pExpInfo
)
{
    Asharp4_result_t res = ASHARP4_RET_SUCCESS;

    int i;
    int iso = 50;
    int iso_div = 50;
    float ratio = 0.0f;
    int iso_low = iso, iso_high = iso;
    int gain_high = 0, gain_low = 0;
    int max_iso_step = RK_SHARP_V4_MAX_ISO_NUM;
    int sum_coeff, offset;

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

#ifndef RK_SIMULATOR_HW
    for (i = 0; i < max_iso_step - 1; i++) {
        if (iso >=  pParams->iso[i]  &&  iso <=  pParams->iso[i + 1] ) {
            iso_low = pParams->iso[i] ;
            iso_high = pParams->iso[i + 1];
            gain_low = i;
            gain_high = i + 1;
            ratio = (float)(iso - iso_low) / (iso_high - iso_low);
            break;
        }
    }

    if(iso < pParams->iso[0] ) {
        iso_low = pParams->iso[0] ;
        iso_high = pParams->iso[1];
        gain_low = 0;
        gain_high = 1;
        ratio = 0;
    }

    if(iso > pParams->iso[max_iso_step - 1] ) {
        iso_low = pParams->iso[max_iso_step - 2] ;
        iso_high = pParams->iso[max_iso_step - 1];
        gain_low = max_iso_step - 2;
        gain_high = max_iso_step - 1;
        ratio = 1;
    }
#else
    for (i = max_iso_step - 1; i >= 0; i--)
    {
        if (iso < iso_div * (2 << i))
        {
            iso_low = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }

    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso)
    {
        iso_high = iso;
        ratio = 0;
    }
    if (iso_high == iso )
    {
        iso_low = iso;
        ratio = 1;
    }
    gain_high = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low = (int)(log((float)iso_low / 50) / log((float)2));


    gain_low = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);
#endif

    LOGD_ASHARP("%s:%d iso:%d gainlow:%d gian_high:%d\n", __FUNCTION__, __LINE__, iso, gain_high, gain_high);

    pSelect->enable = pParams->enable;
    pSelect->pbf_gain = INTERP_V4(pParams->pbf_gain[gain_low], pParams->pbf_gain[gain_high], ratio);
    pSelect->pbf_add = INTERP_V4(pParams->pbf_add[gain_low], pParams->pbf_add[gain_high], ratio);
    pSelect->pbf_ratio = INTERP_V4(pParams->pbf_ratio[gain_low], pParams->pbf_ratio[gain_high], ratio);

    pSelect->gaus_ratio = INTERP_V4(pParams->gaus_ratio[gain_low], pParams->gaus_ratio[gain_high], ratio);
    pSelect->sharp_ratio = INTERP_V4(pParams->sharp_ratio[gain_low], pParams->sharp_ratio[gain_high], ratio);

    pSelect->bf_gain = INTERP_V4(pParams->bf_gain[gain_low], pParams->bf_gain[gain_high], ratio);
    pSelect->bf_add = INTERP_V4(pParams->bf_add[gain_low], pParams->bf_add[gain_high], ratio);
    pSelect->bf_ratio = INTERP_V4(pParams->bf_ratio[gain_low], pParams->bf_ratio[gain_high], ratio);

    for(int i = 0; i < RK_SHARP_V4_PBF_DIAM * RK_SHARP_V4_PBF_DIAM; i++) {
        pSelect->kernel_pre_bila_filter[i] = INTERP_V4(pParams->kernel_pre_bila_filter [gain_low][i], pParams->kernel_pre_bila_filter[gain_high][i], ratio);
    }

    for(int i = 0; i < RK_SHARP_V4_RF_DIAM * RK_SHARP_V4_RF_DIAM; i++) {
        pSelect->kernel_range_filter[i] = INTERP_V4(pParams->kernel_range_filter [gain_low][i], pParams->kernel_range_filter[gain_high][i], ratio);
    }

    for(int i = 0; i < RK_SHARP_V4_BF_DIAM * RK_SHARP_V4_BF_DIAM; i++) {
        pSelect->kernel_bila_filter[i] = INTERP_V4(pParams->kernel_bila_filter [gain_low][i], pParams->kernel_bila_filter[gain_high][i], ratio);
    }

    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++) {
        pSelect->luma_point[i] = pParams->luma_point[i];
        pSelect->luma_sigma[i] = INTERP_V4(pParams->luma_sigma[gain_low][i], pParams->luma_sigma[gain_high][i], ratio);
        pSelect->lum_clip_h[i] = (int16_t)ROUND_F(INTERP_V4(pParams->lum_clip_h[gain_low][i],  pParams->lum_clip_h[gain_high][i], ratio));
        pSelect->ehf_th[i] = (int16_t)ROUND_F(INTERP_V4(pParams->ehf_th[gain_low][i], pParams->ehf_th[gain_high][i], ratio));
    }

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}



Asharp4_result_t sharp_fix_transfer_V4(RK_SHARP_Params_V4_Select_t *pSelect, RK_SHARP_Fix_V4_t* pFix, float fPercent)
{
    int sum_coeff, offset;
    int pbf_sigma_shift = 0;
    int bf_sigma_shift = 0;
    Asharp4_result_t res = ASHARP4_RET_SUCCESS;
    int tmp = 0;

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pSelect == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    if(fPercent <= 0.0) {
        fPercent = 0.000001;
    }

    // SHARP_SHARP_EN (0x0000)
    pFix->sharp_bypass = 0;
    pFix->sharp_en = pSelect->enable;

    // SHARP_SHARP_RATIO  (0x0004)
    tmp = (int)ROUND_F(pSelect->sharp_ratio * fPercent * (1 << rk_sharp_V4_sharp_ratio_fix_bits));
    pFix->sharp_sharp_ratio = CLIP(tmp, 0, 63);
    tmp = (int)ROUND_F(pSelect->bf_ratio / fPercent * (1 << rk_sharp_V4_bf_ratio_fix_bits));
    pFix->sharp_bf_ratio = CLIP(tmp, 0, 128);
    tmp = (int)ROUND_F(pSelect->gaus_ratio / fPercent * (1 << rk_sharp_V4_gaus_ratio_fix_bits));
    pFix->sharp_gaus_ratio = CLIP(tmp, 0, 128);
    tmp = (int)ROUND_F(pSelect->pbf_ratio / fPercent * (1 << rk_sharp_V4_bf_ratio_fix_bits));
    pFix->sharp_pbf_ratio = CLIP(tmp, 0, 128);

    // SHARP_SHARP_LUMA_DX (0x0008)
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM - 1; i++)
    {
        //tmp = (int16_t)(pSelect->luma_point[i]);
        tmp = ( log((float)(pSelect->luma_point[i + 1] - pSelect->luma_point[i])) / log((float)2) );
        pFix->sharp_luma_dx[i] = CLIP(tmp, 0, 15);
    }

    // SHARP_SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    // pre bf sigma inv
    int sigma_deci_bits = 9;
    int sigma_inte_bits = 1;
    int max_val         = 0;
    int min_val         = 65536;
    int shf_bits        = 0;
    short sigma_bits[3];
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++)
    {
        int cur_sigma = FLOOR((pSelect->luma_sigma[i] * pSelect->pbf_gain + pSelect->pbf_add) / fPercent );
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];
    pbf_sigma_shift = sigma_bits[2] - 5;
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++)
    {
        tmp = (int16_t)ROUND_F((float)1 / (pSelect->luma_sigma[i] * pSelect->pbf_gain + pSelect->pbf_add) * fPercent  * (1 << sigma_bits[2]));
        pFix->sharp_pbf_sigma_inv[i] = CLIP(tmp, 0, 1023);
    }

    // SHARP_SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    // bf sigma inv
    sigma_deci_bits = 9;
    sigma_inte_bits = 1;
    max_val         = 0;
    min_val         = 65536;
    shf_bits        = 0;
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++)
    {
        int cur_sigma = FLOOR((pSelect->luma_sigma[i] * pSelect->bf_gain + pSelect->bf_add) / fPercent );
        if(max_val < cur_sigma)
            max_val = cur_sigma;
        if(min_val > cur_sigma)
            min_val = cur_sigma;
    }
    sigma_bits[0] = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1] = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2] = sigma_deci_bits + sigma_bits[0];
    bf_sigma_shift = sigma_bits[2] - 5;
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++)
    {
        tmp = (int16_t)ROUND_F((float)1 / (pSelect->luma_sigma[i] * pSelect->bf_gain + pSelect->bf_add) * fPercent  * (1 << sigma_bits[2]));
        pFix->sharp_bf_sigma_inv[i] = CLIP(tmp, 0, 1023);
    }

    // SHARP_SHARP_SIGMA_SHIFT (0x00024)
    pFix->sharp_pbf_sigma_shift = CLIP(bf_sigma_shift, 0, 15);
    pFix->sharp_bf_sigma_shift = CLIP(pbf_sigma_shift, 0, 15);

    // SHARP_SHARP_EHF_TH_0 (0x0028 -  0x0030)
    // wgt = hf * ehf_th
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++)
    {
        tmp = (int)(pSelect->ehf_th[i] * fPercent);
        pFix->sharp_ehf_th[i] = CLIP(tmp, 0, 1023);
    }

    // SHARP_SHARP_CLIP_HF_0 (0x0034 -  0x003c)
    for(int i = 0; i < RK_SHARP_V4_LUMA_POINT_NUM; i++)
    {
        tmp = (int)(pSelect->lum_clip_h[i] * fPercent);
        pFix->sharp_clip_hf[i] = CLIP(tmp, 0, 1023);
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    // filter coeff
    // bf coeff
    // rk_sharp_V4_pbfCoeff : [4], [1], [0]
    tmp = (int)ROUND_F(pSelect->kernel_pre_bila_filter[0] * (1 << rk_sharp_V4_pbfCoeff_fix_bits));
    pFix->sharp_pbf_coef[0] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_pre_bila_filter[1] * (1 << rk_sharp_V4_pbfCoeff_fix_bits));
    pFix->sharp_pbf_coef[1] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_pre_bila_filter[2] * (1 << rk_sharp_V4_pbfCoeff_fix_bits));
    pFix->sharp_pbf_coef[2] = CLIP(tmp, 0, 127);
    sum_coeff   = pFix->sharp_pbf_coef[0] + 4 * pFix->sharp_pbf_coef[1] + 4 * pFix->sharp_pbf_coef[2];
    offset      = (1 << rk_sharp_V4_pbfCoeff_fix_bits) - sum_coeff;
    tmp = (int)(pFix->sharp_pbf_coef[0] + offset);
    pFix->sharp_pbf_coef[0] = CLIP(tmp, 0, 127);

    // SHARP_SHARP_BF_COEF (0x00044)
    // bf coeff
    // rk_sharp_V4_bfCoeff : [4], [1], [0]
    tmp = (int)ROUND_F(pSelect->kernel_bila_filter[0] * (1 << rk_sharp_V4_hbfCoeff_fix_bits));
    pFix->sharp_bf_coef[0] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_bila_filter[1] * (1 << rk_sharp_V4_hbfCoeff_fix_bits));
    pFix->sharp_bf_coef[1] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_bila_filter[2] * (1 << rk_sharp_V4_hbfCoeff_fix_bits));
    pFix->sharp_bf_coef[2] = CLIP(tmp, 0, 127);
    sum_coeff   = pFix->sharp_bf_coef[0] + 4 * pFix->sharp_bf_coef[1] + 4 * pFix->sharp_bf_coef[2];
    offset      = (1 << rk_sharp_V4_hbfCoeff_fix_bits) - sum_coeff;
    tmp = (int)(pFix->sharp_bf_coef[0] + offset);
    pFix->sharp_bf_coef[0] = CLIP(tmp, 0, 127);

    // SHARP_SHARP_GAUS_COEF (0x00048)
    // rk_sharp_V4_rfCoeff :  [4], [1], [0]
    tmp = (int)ROUND_F(pSelect->kernel_range_filter[0] * (1 << rk_sharp_V4_rfCoeff_fix_bits));
    pFix->sharp_gaus_coef[0] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_range_filter[1] * (1 << rk_sharp_V4_rfCoeff_fix_bits));
    pFix->sharp_gaus_coef[1] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_range_filter[2] * (1 << rk_sharp_V4_rfCoeff_fix_bits));
    pFix->sharp_gaus_coef[2] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_range_filter[3] * (1 << rk_sharp_V4_rfCoeff_fix_bits));
    pFix->sharp_gaus_coef[3] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_range_filter[4] * (1 << rk_sharp_V4_rfCoeff_fix_bits));
    pFix->sharp_gaus_coef[4] = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pSelect->kernel_range_filter[5] * (1 << rk_sharp_V4_rfCoeff_fix_bits));
    pFix->sharp_gaus_coef[5] = CLIP(tmp, 0, 127);
    sum_coeff   = pFix->sharp_gaus_coef[0]
                  + 4 * pFix->sharp_gaus_coef[1]
                  + 4 * pFix->sharp_gaus_coef[2]
                  + 4 * pFix->sharp_gaus_coef[3]
                  + 8 * pFix->sharp_gaus_coef[4]
                  + 4 * pFix->sharp_gaus_coef[5];
    offset = (1 << rk_sharp_V4_rfCoeff_fix_bits) - sum_coeff;
    tmp = (int)(pFix->sharp_gaus_coef[0] + offset);
    pFix->sharp_gaus_coef[0] = CLIP(tmp, 0, 127);


#if 1
    sharp_fix_printf_V4(pFix);
#endif

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}


Asharp4_result_t sharp_fix_printf_V4(RK_SHARP_Fix_V4_t  * pFix)
{
    int i = 0;
    Asharp4_result_t res = ASHARP4_RET_SUCCESS;

    LOGI_ASHARP("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    if(pFix == NULL) {
        LOGI_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    // SHARP_SHARP_EN (0x0000)
    LOGD_ASHARP("(0x0000) sharp_bypass:0x%x sharp_en:0x%x \n",
                pFix->sharp_bypass,
                pFix->sharp_en);


    // SHARP_SHARP_RATIO  (0x0004)
    LOGD_ASHARP("(0x0004) sharp_sharp_ratio:0x%x sharp_bf_ratio:0x%x sharp_gaus_ratio:0x%x sharp_pbf_ratio:0x%x \n",
                pFix->sharp_sharp_ratio,
                pFix->sharp_bf_ratio,
                pFix->sharp_gaus_ratio,
                pFix->sharp_pbf_ratio);

    // SHARP_SHARP_LUMA_DX (0x0008)
    for(int i = 0; i < 7; i++) {
        LOGD_ASHARP("(0x0008) sharp_luma_dx[%d]:0x%x \n",
                    i, pFix->sharp_luma_dx[i]);
    }

    // SHARP_SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    for(int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x000c - 0x0014) sharp_pbf_sigma_inv[%d]:0x%x \n",
                    i, pFix->sharp_pbf_sigma_inv[i]);
    }

    // SHARP_SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    for(int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0018 - 0x0020) sharp_bf_sigma_inv[%d]:0x%x \n",
                    i, pFix->sharp_bf_sigma_inv[i]);
    }

    // SHARP_SHARP_SIGMA_SHIFT (0x00024)
    LOGD_ASHARP("(0x00024) sharp_bf_sigma_shift:0x%x sharp_pbf_sigma_shift:0x%x \n",
                pFix->sharp_bf_sigma_shift,
                pFix->sharp_pbf_sigma_shift);

    // SHARP_SHARP_EHF_TH_0 (0x0028 -  0x0030)
    for(int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0028 - 0x0030) sharp_ehf_th[%d]:0x%x \n",
                    i, pFix->sharp_ehf_th[i]);
    }

    // SHARP_SHARP_CLIP_HF_0 (0x0034 -  0x003c)
    for(int i = 0; i < 8; i++) {
        LOGD_ASHARP("(0x0034 - 0x003c) sharp_clip_hf[%d]:0x%x \n",
                    i, pFix->sharp_clip_hf[i]);
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    for(int i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00040) sharp_pbf_coef[%d]:0x%x \n",
                    i, pFix->sharp_pbf_coef[i]);
    }

    // SHARP_SHARP_BF_COEF (0x00044)
    for(int i = 0; i < 3; i++) {
        LOGD_ASHARP("(0x00044) sharp_bf_coef[%d]:0x%x \n",
                    i, pFix->sharp_bf_coef[i]);
    }

    // SHARP_SHARP_GAUS_COEF (0x00048)
    for(int i = 0; i < 6; i++) {
        LOGD_ASHARP("(0x00048) sharp_gaus_coef[%d]:0x%x \n",
                    i, pFix->sharp_gaus_coef[i]);
    }

    return res;
}




Asharp4_result_t sharp_get_setting_by_name_json_V4(CalibDbV2_SharpV4_t *pCalibdbV2, char *name, int *tuning_idx)
{
    int i = 0;
    Asharp4_result_t res = ASHARP4_RET_SUCCESS;

    LOGI_ASHARP("%s(%d): enter  \n", __FUNCTION__, __LINE__);

    if(pCalibdbV2 == NULL || name == NULL || tuning_idx == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdbV2->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdbV2->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdbV2->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    LOGI_ASHARP("%s:%d snr_name:%s  snr_idx:%d i:%d \n",
                __FUNCTION__, __LINE__,
                name, *tuning_idx, i);
    return res;
}


Asharp4_result_t sharp_init_params_json_V4(RK_SHARP_Params_V4_t *pSharpParams, CalibDbV2_SharpV4_t *pCalibdbV2, int tuning_idx)
{
    Asharp4_result_t res = ASHARP4_RET_SUCCESS;
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;
    CalibDbV2_SharpV4_TuningPara_Setting_ISO_t *pTuningISO;

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pSharpParams == NULL || pCalibdbV2 == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }


    for(i = 0; i < pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_SHARP_V4_MAX_ISO_NUM; i++) {
        pTuningISO = &pCalibdbV2->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
        pSharpParams->iso[i] = pTuningISO->iso;

        for(j = 0; j < RK_SHARP_V4_LUMA_POINT_NUM; j++) {
            pSharpParams->luma_point[j] = pTuningISO->luma_para.luma_point[j];
            pSharpParams->luma_sigma[i][j] = pTuningISO->luma_para.luma_sigma[j];
            pSharpParams->lum_clip_h[i][j] = pTuningISO->luma_para.hf_clip[j];
            pSharpParams->ehf_th[i][j] = pTuningISO->luma_para.local_sharp_strength[j];
        }

        pSharpParams->pbf_gain[i] = pTuningISO->pbf_gain;
        pSharpParams->pbf_add[i] = pTuningISO->pbf_add;
        pSharpParams->pbf_ratio[i] = pTuningISO->pbf_ratio;
        pSharpParams->gaus_ratio[i] = pTuningISO->gaus_ratio;
        pSharpParams->sharp_ratio[i] = pTuningISO->sharp_ratio;
        pSharpParams->bf_gain[i] = pTuningISO->bf_gain;
        pSharpParams->bf_add[i] = pTuningISO->bf_add;
        pSharpParams->bf_ratio[i] = pTuningISO->bf_ratio;

        for(j = 0; j < 3; j++) {
            pSharpParams->kernel_pre_bila_filter[i][j] = pTuningISO->kernel_para.prefilter_coeff[j];
            pSharpParams->kernel_bila_filter[i][j] = pTuningISO->kernel_para.hfBilateralFilter_coeff[j];
            LOGD_ASHARP("kernel: index[%d][%d] = %f\n", i, j, pSharpParams->kernel_bila_filter[i][j]);
        }

        for(j = 0; j < 6; j++) {
            pSharpParams->kernel_range_filter[i][j] = pTuningISO->kernel_para.GaussianFilter_coeff[j];
            LOGD_ASHARP("kernel: index[%d][%d] = %f\n", i, j, pSharpParams->kernel_range_filter[i][j]);
        }
    }

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);
    return res;
}


Asharp4_result_t sharp_config_setting_param_json_V4(RK_SHARP_Params_V4_t *pParams, CalibDbV2_SharpV4_t *pCalibdbV2, char* param_mode, char * snr_name)
{
    Asharp4_result_t res = ASHARP4_RET_SUCCESS;
    int tuning_idx = 0;

    LOGI_ASHARP("%s(%d): enter\n", __FUNCTION__, __LINE__);

    if(pParams == NULL || pCalibdbV2 == NULL || param_mode == NULL || snr_name == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ASHARP4_RET_NULL_POINTER;
    }

    LOGD_ASHARP("%s(%d): mode:%s  setting:%s \n", __FUNCTION__, __LINE__, param_mode, snr_name);

    res = sharp_get_setting_by_name_json_V4(pCalibdbV2, snr_name, &tuning_idx);
    if(res != ASHARP4_RET_SUCCESS) {
        LOGW_ASHARP("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = sharp_init_params_json_V4(pParams, pCalibdbV2, tuning_idx);
    pParams->enable = pCalibdbV2->TuningPara.enable;

    LOGI_ASHARP("%s(%d): exit\n", __FUNCTION__, __LINE__);

    return res;

}

RKAIQ_END_DECLARE


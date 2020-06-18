
#include "rk_aiq_anr_algo.h"
#include "rk_aiq_algo_anr_itf.h"

RKAIQ_BEGIN_DECLARE

//anr inint
ANRresult_t ANRInit(ANRContext_t **ppANRCtx, CamCalibDbContext_t *pCalibDb)
{
    ANRContext_t * pANRCtx;

    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pANRCtx = (ANRContext_t *)malloc(sizeof(ANRContext_t));
    if(pANRCtx == NULL) {
        LOGE_ANR("%s(%d): malloc fail\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    memset(pANRCtx, 0x00, sizeof(ANRContext_t));

    //gain state init
    pANRCtx->stGainState.gainState = -1;
    pANRCtx->stGainState.gain_th0 = 32.0;
    pANRCtx->stGainState.gain_th1 = 128.0;

    pANRCtx->eState = ANR_STATE_INITIALIZED;
    *ppANRCtx = pANRCtx;

    pANRCtx->refYuvBit = 8;
    pANRCtx->eMode = ANR_OP_MODE_AUTO;

#if ANR_USE_XML_FILE
    //read v1 params from xml
    pANRCtx->stBayernrCalib = pCalibDb->bayerNr;
    pANRCtx->stUvnrCalib = pCalibDb->uvnr;
    pANRCtx->stYnrCalib = pCalibDb->ynr;
    pANRCtx->stMfnrCalib = pCalibDb->mfnr;
#endif

#if RK_SIMULATOR_HW
    //just for v2 params from html
    FILE *fp2 = fopen("rkaiq_anr_html_params.bin", "rb");
    if(fp2 != NULL) {
        memset(&pANRCtx->stAuto.stBayernrParams, 0, sizeof(RKAnr_Bayernr_Params_t));
        fread(&pANRCtx->stAuto.stBayernrParams, 1, sizeof(RKAnr_Bayernr_Params_t), fp2);
        memset(&pANRCtx->stAuto.stMfnrParams, 0, sizeof(RKAnr_Mfnr_Params_t));
        fread(&pANRCtx->stAuto.stMfnrParams, 1, sizeof(RKAnr_Mfnr_Params_t), fp2);
        memset(&pANRCtx->stAuto.stUvnrParams, 0, sizeof(RKAnr_Uvnr_Params_t));
        fread(&pANRCtx->stAuto.stUvnrParams, 1, sizeof(RKAnr_Uvnr_Params_t), fp2);
        memset(&pANRCtx->stAuto.stYnrParams, 0, sizeof(RKAnr_Ynr_Params_t));
        fread(&pANRCtx->stAuto.stYnrParams, 1, sizeof(RKAnr_Ynr_Params_t), fp2);
        LOGD_ANR("oyyf: %s:(%d) read anr param html file sucess! \n", __FUNCTION__, __LINE__);
    } else {
        LOGE_ANR("oyyf: %s:(%d) read anr param html file failed! \n", __FUNCTION__, __LINE__);
        return ANR_RET_FAILURE;
    }
#endif

#if ANR_USE_XML_FILE
	pANRCtx->stExpInfo.snr_mode = 1;
	ANRConfigSettingParam(pANRCtx, pANRCtx->stExpInfo.snr_mode);
#endif

    LOGD_ANR("%s(%d): bayernr %f %f %f %d %d %f", __FUNCTION__, __LINE__,
             pANRCtx->stAuto.stBayernrParams.filtpar[0],
             pANRCtx->stAuto.stBayernrParams.filtpar[4],
             pANRCtx->stAuto.stBayernrParams.filtpar[8],
             pANRCtx->stAuto.stBayernrParams.peaknoisesigma,
             pANRCtx->stAuto.stBayernrParams.sw_bayernr_edge_filter_en,
             pANRCtx->stAuto.stBayernrParams.sw_bayernr_filter_strength[0]);


    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;
}

//anr release
ANRresult_t ANRRelease(ANRContext_t *pANRCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pANRCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    memset(pANRCtx, 0x00, sizeof(ANRContext_t));
    free(pANRCtx);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;
}

//anr config
ANRresult_t ANRPrepare(ANRContext_t *pANRCtx, ANRConfig_t* pANRConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pANRCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pANRConfig == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    //pANRCtx->eMode = pANRConfig->eMode;
    //pANRCtx->eState = pANRConfig->eState;
    //pANRCtx->refYuvBit = pANRConfig->refYuvBit;

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;
}

//anr reconfig
ANRresult_t ANRReConfig(ANRContext_t *pANRCtx, ANRConfig_t* pANRConfig)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;
}

//anr preprocess
ANRresult_t ANRPreProcess(ANRContext_t *pANRCtx)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    //need todo what?

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;
}

//anr process
ANRresult_t ANRProcess(ANRContext_t *pANRCtx, ANRExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pANRCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pANRCtx->eMode == ANR_OP_MODE_AUTO) {

        LOGD_ANR("%s(%d): refYuvBit:%d\n", __FUNCTION__, __LINE__, pANRCtx->refYuvBit);

		#if ANR_USE_XML_FILE
		if(pExpInfo->snr_mode != pANRCtx->stExpInfo.snr_mode){
			ANRConfigSettingParam(pANRCtx, pExpInfo->snr_mode);
		}
		#endif
        memcpy(&pANRCtx->stExpInfo, pExpInfo, sizeof(ANRExpInfo_t));

        ANRGainRatioProcess(&pANRCtx->stGainState, &pANRCtx->stExpInfo);
        //select param
        select_bayernr_params_by_ISO(&pANRCtx->stAuto.stBayernrParams, &pANRCtx->stAuto.stBayernrParamSelect, pExpInfo);
        select_mfnr_params_by_ISO(&pANRCtx->stAuto.stMfnrParams, &pANRCtx->stAuto.stMfnrParamSelect, pExpInfo, pANRCtx->refYuvBit);
        select_ynr_params_by_ISO(&pANRCtx->stAuto.stYnrParams, &pANRCtx->stAuto.stYnrParamSelect, pExpInfo, pANRCtx->refYuvBit);
        select_uvnr_params_by_ISO(&pANRCtx->stAuto.stUvnrParams, &pANRCtx->stAuto.stUvnrParamSelect, pExpInfo);

    } else if(pANRCtx->eMode == ANR_OP_MODE_MANUAL) {
        //TODO
    }

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;

}

ANRresult_t ANRSetGainMode(ANRProcResult_t* pANRResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pANRResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pANRResult->stGainFix.gain_table_en) {
        pANRResult->stMfnrFix.gain_en = 0;
        pANRResult->stUvnrFix.nr_gain_en = 0;

    } else {
        pANRResult->stMfnrFix.gain_en = 1;
        pANRResult->stUvnrFix.nr_gain_en = 1;
    }

    return ANR_RET_SUCCESS;
}


//anr get result
ANRresult_t ANRGetProcResult(ANRContext_t *pANRCtx, ANRProcResult_t* pANRResult)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pANRCtx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pANRResult == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pANRCtx->eMode == ANR_OP_MODE_AUTO) {

        pANRResult->stBayernrParamSelect = pANRCtx->stAuto.stBayernrParamSelect;
        pANRResult->stMfnrParamSelect = pANRCtx->stAuto.stMfnrParamSelect;
        pANRResult->stYnrParamSelect = pANRCtx->stAuto.stYnrParamSelect;
        pANRResult->stUvnrParamSelect = pANRCtx->stAuto.stUvnrParamSelect;

        pANRResult->bayernrEn = pANRCtx->stAuto.bayernrEn;
        pANRResult->mfnrEn = pANRCtx->stAuto.mfnrEn;
        pANRResult->ynrEN = pANRCtx->stAuto.ynrEn;
        pANRResult->uvnrEn = pANRCtx->stAuto.uvnrEn;


    } else if(pANRCtx->eMode == ANR_OP_MODE_MANUAL) {
        //TODO
        pANRResult->bayernrEn = pANRCtx->stManual.bayernrEn;
        pANRResult->stBayernrParamSelect = pANRCtx->stManual.stBayernrParamSelect;
        pANRResult->mfnrEn = pANRCtx->stManual.mfnrEn;
        pANRResult->stMfnrParamSelect = pANRCtx->stManual.stMfnrParamSelect;
        pANRResult->ynrEN = pANRCtx->stManual.ynrEn;
        pANRResult->stYnrParamSelect = pANRCtx->stManual.stYnrParamSelect;
        pANRResult->uvnrEn = pANRCtx->stManual.uvnrEn;
        pANRResult->stUvnrParamSelect = pANRCtx->stManual.stUvnrParamSelect;
    }

    //transfer to reg value
    bayernr_fix_tranfer(&pANRResult->stBayernrParamSelect, &pANRResult->stBayernrFix);
    mfnr_fix_transfer(&pANRResult->stMfnrParamSelect, &pANRResult->stMfnrFix, &pANRCtx->stExpInfo, pANRCtx->stGainState.ratio);
    ynr_fix_transfer(&pANRResult->stYnrParamSelect, &pANRResult->stYnrFix);
    uvnr_fix_transfer(&pANRResult->stUvnrParamSelect, &pANRResult->stUvnrFix, &pANRCtx->stExpInfo, pANRCtx->stGainState.ratio);
    gain_fix_transfer(&pANRResult->stMfnrParamSelect, &pANRResult->stGainFix, &pANRCtx->stExpInfo, pANRCtx->stGainState.ratio);
    pANRResult->stBayernrFix.rawnr_en = pANRResult->bayernrEn;
    pANRResult->stMfnrFix.tnr_en = pANRResult->mfnrEn;
    pANRResult->stMfnrFix.mode = pANRCtx->stMfnrCalib.mode;
    pANRResult->stYnrFix.ynr_en = pANRResult->ynrEN;
    pANRResult->stUvnrFix.uvnr_en = pANRResult->uvnrEn;
    pANRResult->stGainFix.gain_table_en = pANRCtx->stMfnrCalib.local_gain_en;
    ANRSetGainMode(pANRResult);

    LOGD_ANR("%s:%d xml:local:%d mode:%d  reg: local gain:%d  mfnr gain:%d mode:%d\n",
             __FUNCTION__, __LINE__,
             pANRCtx->stMfnrCalib.local_gain_en,
             pANRCtx->stMfnrCalib.mode,
             pANRResult->stGainFix.gain_table_en,
             pANRResult->stMfnrFix.gain_en,
             pANRResult->stMfnrFix.mode);
    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ANR_RET_SUCCESS;
}

ANRresult_t ANRGainRatioProcess(ANRGainState_t *pGainState, ANRExpInfo_t *pExpInfo)
{
    LOGI_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    float th;
    float ratio;

    if(pGainState == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }

    int gain_stat = pGainState->gainState;
    float gain_th0 = pGainState->gain_th0;
    float gain_th1 = pGainState->gain_th1;
    float gain_cur = pExpInfo->arAGain[pExpInfo->hdr_mode] * pExpInfo->arDGain[pExpInfo->hdr_mode];

    pGainState->gain_cur = gain_cur;

    if(gain_stat == -1)
    {
        th = (gain_th0 + gain_th1) / 2;
        if(gain_cur > th)
            gain_stat = 1;
        else
            gain_stat = 0;
    }
    else if(gain_stat == 0)
    {
        if(gain_cur > gain_th1)
            gain_stat = 1;
    }
    else if(gain_stat == 1)
    {
        if(gain_cur < gain_th0)
            gain_stat = 0;
    }
    else {
        LOGE_ANR("%s:%d invalid stat\n", __FUNCTION__, __LINE__);
        return ANR_RET_INVALID_PARM;
    }


    if(gain_stat == 0)
        pGainState->ratio = 1;
    else
        pGainState->ratio = 1.0 / 16;

    LOGD_ANR("%s:%d gain_cur:%f th: %f %f ratio:%f \n",
             __FUNCTION__, __LINE__,
             gain_cur,
             gain_th0,
             gain_th1,
             pGainState->ratio);

    LOGI_ANR("%s(%d): exit!\n", __FUNCTION__, __LINE__);

    return ANR_RET_SUCCESS;
}

ANRresult_t ANRConfigSettingParam(ANRContext_t *pANRCtx, int snr_mode)
{
      char snr_name[CALIBDB_NR_SHARP_NAME_LENGTH];
	memset(snr_name, 0x00, sizeof(snr_name));
	
	 if(pANRCtx == NULL) {
        	LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        	return ANR_RET_INVALID_PARM;
    	}
	 
	if(snr_mode == 1){
		sprintf(snr_name, "%s", "HSNR");
	}else if(snr_mode == 0){
		sprintf(snr_name, "%s", "LSNR");
	}else{
		LOGE_ANR("%s(%d): not support snr mode!\n", __FUNCTION__, __LINE__);
		sprintf(snr_name, "%s", "HSNR");
	}
	
	pANRCtx->stAuto.bayernrEn = pANRCtx->stBayernrCalib.enable;
	bayernr_config_setting_param(&pANRCtx->stAuto.stBayernrParams, &pANRCtx->stBayernrCalib, snr_name);

	pANRCtx->stAuto.uvnrEn = pANRCtx->stUvnrCalib.enable;
	uvnr_config_setting_param(&pANRCtx->stAuto.stUvnrParams, &pANRCtx->stUvnrCalib, snr_name);
	
	pANRCtx->stAuto.ynrEn = pANRCtx->stYnrCalib.enable;
	ynr_config_setting_param(&pANRCtx->stAuto.stYnrParams, &pANRCtx->stYnrCalib, snr_name);
	
	pANRCtx->stAuto.mfnrEn = pANRCtx->stMfnrCalib.enable;
	mfnr_config_setting_param(&pANRCtx->stAuto.stMfnrParams, &pANRCtx->stMfnrCalib, snr_name);

	return ANR_RET_SUCCESS; 
}
RKAIQ_END_DECLARE



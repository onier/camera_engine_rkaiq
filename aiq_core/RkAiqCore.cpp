/*
 * rkisp_aiq_core.h
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

#include "RkAiqHandleInt.h"
#include "RkAiqCore.h"
#include "v4l2_buffer_proxy.h"
#include "acp/rk_aiq_algo_acp_itf.h"
#include "ae/rk_aiq_algo_ae_itf.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "af/rk_aiq_algo_af_itf.h"
#include "anr/rk_aiq_algo_anr_itf.h"
#include "asd/rk_aiq_algo_asd_itf.h"
#include "ahdr/rk_aiq_algo_ahdr_itf.h"
#include "asharp/rk_aiq_algo_asharp_itf.h"
#include "adehaze/rk_aiq_algo_adhaz_itf.h"
#include "ablc/rk_aiq_algo_ablc_itf.h"
#include "adpcc/rk_aiq_algo_adpcc_itf.h"
#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "agic/rk_aiq_algo_agic_itf.h"
#include "adebayer/rk_aiq_algo_adebayer_itf.h"
#include "accm/rk_aiq_algo_accm_itf.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "awdr/rk_aiq_algo_awdr_itf.h"
#include "a3dlut/rk_aiq_algo_a3dlut_itf.h"
#include "aldch/rk_aiq_algo_aldch_itf.h"
#include "ar2y/rk_aiq_algo_ar2y_itf.h"
#include "aie/rk_aiq_algo_aie_itf.h"
#include "aorb/rk_aiq_algo_aorb_itf.h"
#include "afec/rk_aiq_algo_afec_itf.h"
#include "acgc/rk_aiq_algo_acgc_itf.h"
#ifdef RK_SIMULATOR_HW
#include "simulator/isp20_hw_simulator.h"
#else
#include "isp20/Isp20StatsBuffer.h"
#include "isp20/rkisp2-config.h"
#endif

namespace RkCam {

/*
 * notice that the order should be the same as enum RkAiqAlgoType_t
 * which defined in rk_aiq/algos/rk_aiq_algo_des.h
 */
static RkAiqAlgoDesComm* g_default_3a_des[] = {
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAe,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAwb,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAf,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAblc,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAdpcc,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAhdr,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAnr,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAlsc,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAgic,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAdebayer,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAccm,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAgamma,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAwdr,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAdhaz,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescA3dlut,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAldch,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAr2y,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAcp,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAie,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAsharp,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAorb,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAfec,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAcgc,
    (RkAiqAlgoDesComm*)&g_RkIspAlgoDescAsd,
};

bool
RkAiqCoreThread::loop()
{
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<VideoBuffer> stats = mStatsQueue.pop (timeout);

    if (!stats.ptr()) {
        LOGW_ANALYZER("RkAiqCoreThread got empty stats, stop thread");
        return false;
    }

    XCamReturn ret = mRkAiqCore->analyze (stats);
    if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_BYPASS)
        return true;

    LOGE_ANALYZER("RkAiqCoreThread failed to analyze 3a stats");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

uint16_t RkAiqCore::DEFAULT_POOL_SIZE = 20;

RkAiqCore::RkAiqCore()
    : mRkAiqCoreTh(new RkAiqCoreThread(this))
    , mRkAiqCorePpTh(new RkAiqCoreThread(this))
    , mState(RK_AIQ_CORE_STATE_INVALID)
    , mCb(NULL)
    , mAiqParamsPool(new RkAiqFullParamsPool("RkAiqFullParams", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqExpParamsPool(new RkAiqExpParamsPool("RkAiqExpParams", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqIspParamsPool(new RkAiqIspParamsPool("RkAiqIspParams", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqIsppParamsPool(new RkAiqIsppParamsPool("RkAiqIspParams", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqFocusParamsPool(new RkAiqFocusParamsPool("RkAiqFocusParams", RkAiqCore::DEFAULT_POOL_SIZE))
{
    ENTER_ANALYZER_FUNCTION();
    mAlogsSharedParams.reset();
    mCurAhdrAlgoHdl = NULL;
    mCurAnrAlgoHdl = NULL;
    mCurAdhazAlgoHdl = NULL;
    mCurAsdAlgoHdl = NULL;
    mCurAcpAlgoHdl = NULL;
    mCurAsharpAlgoHdl = NULL;
    mCurA3dlutAlgoHdl = NULL;
    mCurAblcAlgoHdl = NULL;
    mCurAccmAlgoHdl = NULL;
    mCurAcgcAlgoHdl = NULL;
    mCurAdebayerAlgoHdl = NULL;
    mCurAdpccAlgoHdl = NULL;
    mCurAfecAlgoHdl = NULL;
    mCurAgammaAlgoHdl = NULL;
    mCurAgicAlgoHdl = NULL;
    mCurAieAlgoHdl = NULL;
    mCurAldchAlgoHdl = NULL;
    mCurAlscAlgoHdl = NULL;
    mCurAorbAlgoHdl = NULL;
    mCurAr2yAlgoHdl = NULL;
    mCurAwdrAlgoHdl = NULL;
    mCurAeAlgoHdl = NULL;
    mCurAwbAlgoHdl = NULL;
    mCurAfAlgoHdl = NULL;

    SmartPtr<RkAiqFullParams> fullParam = new RkAiqFullParams();
    mAiqCurParams = new RkAiqFullParamsProxy(fullParam );

    EXIT_ANALYZER_FUNCTION();
}

RkAiqCore::~RkAiqCore()
{
    ENTER_ANALYZER_FUNCTION();
    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqCore::init(const char* sns_ent_name, const CamCalibDbContext_t* aiqCalib)
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_INVALID) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    mAlogsSharedParams.calib = aiqCalib;

    addDefaultAlgos();

    mState = RK_AIQ_CORE_STATE_INITED;
    return XCAM_RETURN_NO_ERROR;

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqCore::deInit()
{
    ENTER_ANALYZER_FUNCTION();

    if (mState == RK_AIQ_CORE_STATE_STARTED) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    mState = RK_AIQ_CORE_STATE_INVALID;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::start()
{
    ENTER_ANALYZER_FUNCTION();

    if ((mState != RK_AIQ_CORE_STATE_PREPARED) &&
            (mState != RK_AIQ_CORE_STATE_STOPED)) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    mRkAiqCoreTh->start();
    mRkAiqCorePpTh->start();
    mState = RK_AIQ_CORE_STATE_STARTED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::stop()
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_STARTED) {
        LOGW_ANALYZER("in state %d\n", mState);
        return XCAM_RETURN_NO_ERROR;
    }

    mRkAiqCoreTh->triger_stop();
    mRkAiqCoreTh->stop();
    mRkAiqCorePpTh->triger_stop();
    mRkAiqCorePpTh->stop();
    mState = RK_AIQ_CORE_STATE_STOPED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::prepare(const rk_aiq_exposure_sensor_descriptor* sensor_des,
                   int mode)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // check state
    if ((mState == RK_AIQ_CORE_STATE_STARTED) ||
            (mState == RK_AIQ_CORE_STATE_INVALID)) {
        LOGW_ANALYZER("in state %d\n", mState);
        return XCAM_RETURN_NO_ERROR;
    }

    mAlogsSharedParams.snsDes = *sensor_des;
    mAlogsSharedParams.working_mode = mode;

    // for not hdr mode
    if (mAlogsSharedParams.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        enableAlgo(RK_AIQ_ALGO_TYPE_AHDR, 0, false);

#define PREPARE_ALGO(at) \
    LOGD_ANALYZER("%s handle prepare start ....", #at); \
    if (mCur##at##AlgoHdl.ptr() && mCur##at##AlgoHdl->getEnable()) { \
        /* update user initial params */ \
        ret = mCur##at##AlgoHdl->updateConfig(); \
        RKAIQCORE_CHECK_BYPASS(ret, "%s update initial user params failed", #at); \
        ret = mCur##at##AlgoHdl->prepare(); \
        RKAIQCORE_CHECK_BYPASS(ret, "%s prepare failed", #at); \
    } \
    LOGD_ANALYZER("%s handle prepare end ....", #at);

    PREPARE_ALGO(Ae);
    PREPARE_ALGO(Awb);
    PREPARE_ALGO(Af);
    PREPARE_ALGO(Ahdr);
    PREPARE_ALGO(Anr);
    PREPARE_ALGO(Adhaz);
    PREPARE_ALGO(Acp);
    PREPARE_ALGO(Asharp);
    PREPARE_ALGO(A3dlut);
    PREPARE_ALGO(Ablc);
    PREPARE_ALGO(Accm);
    PREPARE_ALGO(Acgc);
    PREPARE_ALGO(Adebayer);
    PREPARE_ALGO(Adpcc);
    PREPARE_ALGO(Afec);
    PREPARE_ALGO(Agamma);
    PREPARE_ALGO(Agic);
    PREPARE_ALGO(Aie);
    PREPARE_ALGO(Aldch);
    PREPARE_ALGO(Alsc);
    PREPARE_ALGO(Aorb);
    PREPARE_ALGO(Ar2y);
    PREPARE_ALGO(Awdr);
    PREPARE_ALGO(Asd);

    mAlogsSharedParams.init = true;
    analyzeInternal();
    analyzeInternalPp();

    mState = RK_AIQ_CORE_STATE_PREPARED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<RkAiqFullParamsProxy>
RkAiqCore::analyzeInternal()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mAlogsSharedParams.init) {
        // run algos without stats to generate
        // initial params
        mAlogsSharedParams.ispStats.aec_stats_valid = false;
        mAlogsSharedParams.ispStats.awb_stats_valid = false;
        mAlogsSharedParams.ispStats.af_stats_valid = false;
        mAlogsSharedParams.ispStats.ahdr_stats_valid = false;
    }

    SmartPtr<RkAiqFullParamsProxy> aiqParamProxy = mAiqParamsPool->get_item();

    if (!aiqParamProxy.ptr()) {
        LOGE_ANALYZER("no free aiq params buffer!");
        return NULL;
    }

    RkAiqFullParams* aiqParams = aiqParamProxy->data().ptr();
    aiqParams->reset();

    if (mAiqIspParamsPool->has_free_items()) {
        aiqParams->mIspParams = mAiqIspParamsPool->get_item();
    } else {
        LOGE_ANALYZER("no free isp params buffer!");
        return NULL;
    }

    if (mAiqExpParamsPool->has_free_items()) {
        aiqParams->mExposureParams = mAiqExpParamsPool->get_item();
    } else {
        LOGE_ANALYZER("no free exposure params buffer!");
        return NULL;
    }

    if (mAiqFocusParamsPool->has_free_items()) {
        aiqParams->mFocusParams = mAiqFocusParamsPool->get_item();
    } else {
        LOGE_ANALYZER("no free focus params buffer!");
        return NULL;
    }

    if (mAiqIsppParamsPool->has_free_items()) {
        aiqParams->mIsppParams = mAiqIsppParamsPool->get_item();
    } else {
        LOGE_ANALYZER("no free ispp params buffer!");
        return NULL;
    }

    ret = preProcess();
    RKAIQCORE_CHECK_RET_NULL(ret, "preprocess failed");

    ret = processing();
    RKAIQCORE_CHECK_RET_NULL(ret, "processing failed");

    ret = postProcess();
    RKAIQCORE_CHECK_RET_NULL(ret, "post process failed");

    genIspAeResult(aiqParams);
    genIspAwbResult(aiqParams);
    genIspAfResult(aiqParams);
    genIspAhdrResult(aiqParams);
    genIspAnrResult(aiqParams);
    genIspAdhazResult(aiqParams);
    genIspAsdResult(aiqParams);
    genIspAcpResult(aiqParams);
    genIspAieResult(aiqParams);
    genIspAsharpResult(aiqParams);
    genIspA3dlutResult(aiqParams);
    genIspAblcResult(aiqParams);
    genIspAccmResult(aiqParams);
    genIspAcgcResult(aiqParams);
    genIspAdebayerResult(aiqParams);
    genIspAdpccResult(aiqParams);
    genIspAfecResult(aiqParams);
    genIspAgammaResult(aiqParams);
    genIspAgicResult(aiqParams);
    genIspAldchResult(aiqParams);
    genIspAlscResult(aiqParams);
    genIspAr2yResult(aiqParams);
    genIspAwdrResult(aiqParams);

    mAiqCurParams->data()->mIspParams = aiqParams->mIspParams;
    mAiqCurParams->data()->mExposureParams = aiqParams->mExposureParams;
    mAiqCurParams->data()->mFocusParams = aiqParams->mFocusParams;
    mAiqCurParams->data()->mIsppParams = aiqParams->mIsppParams;

    EXIT_ANALYZER_FUNCTION();

    return aiqParamProxy;
}

SmartPtr<RkAiqFullParamsProxy>
RkAiqCore::analyzeInternalPp()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mAlogsSharedParams.init) {
        // run algos without stats to generate
        // initial params
        mAlogsSharedParams.ispStats.orb_stats_valid = false;
    }

    SmartPtr<RkAiqFullParamsProxy> aiqParamProxy = mAiqParamsPool->get_item();

    if (!aiqParamProxy.ptr()) {
        LOGE_ANALYZER("no free aiq params buffer!");
        return NULL;
    }

    RkAiqFullParams* aiqParams = aiqParamProxy->data().ptr();
    aiqParams->reset();

    if (mAiqIsppParamsPool->has_free_items()) {
        aiqParams->mIsppParams = mAiqIsppParamsPool->get_item();
        if (!aiqParams->mIsppParams.ptr()) {
            LOGE_ANALYZER("no free ispp params buffer!");
            return NULL;
        }
    } else {
        LOGE_ANALYZER("no free ispp params buffer!");
        return NULL;
    }

    ret = preProcessPp();
    RKAIQCORE_CHECK_RET_NULL(ret, "preprocessPp failed");

    ret = processingPp();
    RKAIQCORE_CHECK_RET_NULL(ret, "processingPp failed");

    ret = postProcessPp();
    RKAIQCORE_CHECK_RET_NULL(ret, "post processPp failed");

    genIspAorbResult(aiqParams);

    if (!mAiqCurParams->data()->mIsppParams.ptr())
        mAiqCurParams->data()->mIsppParams = aiqParams->mIsppParams;
    else
        mAiqCurParams->data()->mIsppParams->data()->orb =
            aiqParams->mIsppParams->data()->orb;

    EXIT_ANALYZER_FUNCTION();

    return aiqParamProxy;
}

XCamReturn
RkAiqCore::genIspAeResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAe* ae_com =
        mAlogsSharedParams.procResComb.ae_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();
    SmartPtr<rk_aiq_exposure_params_t> exp_param =
        params->mExposureParams->data();

    if (!ae_com) {
        LOGD_ANALYZER("no ae result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ae common result
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AE);
    int algo_id = (*handle)->getAlgoId();
    // gen common result

    exp_param->LinearExp = ae_com->new_ae_exp.LinearExp;
    memcpy(exp_param->HdrExp, ae_com->new_ae_exp.HdrExp, sizeof(ae_com->new_ae_exp.HdrExp));
    exp_param->frame_length_lines = ae_com->new_ae_exp.frame_length_lines;
    exp_param->line_length_pixels = ae_com->new_ae_exp.line_length_pixels;
    exp_param->pixel_clock_freq_mhz = ae_com->new_ae_exp.pixel_clock_freq_mhz;

    isp_param->aec_meas = ae_com->ae_meas;
    isp_param->hist_meas = ae_com->hist_meas;

    LOGD_ANALYZER("%s:genIspAeResult test,rawae1.wnd_num=%d,rawae2.win,h_size=%d,rawhist2.weight0=%d", __FUNCTION__,
                  isp_param->aec_meas.rawae1.wnd_num, isp_param->aec_meas.rawae2.win.h_size,
                  isp_param->hist_meas.rawhist1.weight[0]);


    // gen rk ae result
    if (algo_id == 0) {
        RkAiqAlgoProcResAeInt* ae_rk = (RkAiqAlgoProcResAeInt*)ae_com;
        memcpy(exp_param->exp_tbl, ae_rk->ae_proc_res_rk.exp_set_tbl, sizeof(exp_param->exp_tbl));
        exp_param->exp_cnt = ae_rk->ae_proc_res_rk.exp_set_cnt;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAwbResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAwb* awb_com =
        mAlogsSharedParams.procResComb.awb_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!awb_com) {
        LOGD_ANALYZER("no awb result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen awb common result
    RkAiqAlgoProcResAwb* awb_rk = (RkAiqAlgoProcResAwb*)awb_com;
    isp_param->awb_gain = awb_rk->awb_gain_algo;
    isp_param->awb_cfg_v200 = awb_rk->awb_hw0_para;
    isp_param->awb_cfg_v201 = awb_rk->awb_hw1_para;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AWB);
    int algo_id = (*handle)->getAlgoId();

    // gen rk awb result
    if (algo_id == 0) {
        RkAiqAlgoProcResAwbInt* awb_rk_int = (RkAiqAlgoProcResAwbInt*)awb_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAfResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAf* af_com =
        mAlogsSharedParams.procResComb.af_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();
    SmartPtr<rk_aiq_focus_params_t> focus_param =
        params->mFocusParams->data();

    if (!af_com) {
        LOGD_ANALYZER("no af result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen af common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AF);
    int algo_id = (*handle)->getAlgoId();

    // gen rk af result
    if (algo_id == 0) {
        RkAiqAlgoProcResAfInt* af_rk = (RkAiqAlgoProcResAfInt*)af_com;

        isp_param->af_meas = af_rk->af_proc_res_com.af_isp_param;

        focus_param->next_lens_pos = af_rk->af_proc_res_com.af_focus_param.next_lens_pos;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAhdrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAhdr* ahdr_com =
        mAlogsSharedParams.procResComb.ahdr_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!ahdr_com) {
        LOGD_ANALYZER("no ahdr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ahdr common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AHDR);
    int algo_id = (*handle)->getAlgoId();

    // gen rk ahdr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAhdrInt* ahdr_rk = (RkAiqAlgoProcResAhdrInt*)ahdr_com;

        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_mode =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_mode;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_lm_dif_0p9 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_lm_dif_0p9;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_ms_dif_0p8 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_ms_dif_0p8;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_lm_dif_0p15 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_lm_dif_0p15;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_ms_dif_0p15 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_ms_dif_0p15;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_gain0 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_gain0;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_gain0_inv =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_gain0_inv;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_gain1 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_gain1;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_gain1_inv =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_gain1_inv;
        isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_gain2 =
            ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_gain2;
        for(int i = 0; i < 17; i++)
        {
            isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_e_y[i] =
                ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_e_y[i];
            isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_l1_y[i] =
                ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_l1_y[i];
            isp_param->ahdr_proc_res.MgeProcRes.sw_hdrmge_l0_y[i] =
                ahdr_rk->AhdrProcRes.MgeProcRes.sw_hdrmge_l0_y[i];
        }

        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_lgmax =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_lgmax;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_lgscl =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_lgscl;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_lgscl_inv =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_lgscl_inv;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_clipratio0 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_clipratio0;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_clipratio1 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_clipratio1;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_clipgap0 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_clipgap0;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_clipgap1 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_clipgap1;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_ratiol =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_ratiol;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_hist_min =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_hist_min;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_hist_low =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_hist_low;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_hist_high =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_hist_high;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_hist_0p3 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_hist_0p3;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_hist_shift =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_hist_shift;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_palpha_0p18 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_palpha_0p18;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_palpha_lw0p5 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_palpha_lw0p5;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_palpha_lwscl =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_palpha_lwscl;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_maxpalpha =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_maxpalpha;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_maxgain =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_maxgain;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_cfg_alpha =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_cfg_alpha;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_gainoff =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_gainoff;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_lgmin =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_lgmin;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_lgmax =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_lgmax;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_lgmean =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_lgmean;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_weightkey =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_weightkey;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_lgrange0 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_lgrange0;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_lgrange1 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_lgrange1;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_lgavgmax =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_lgavgmax;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_set_palpha =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_set_palpha;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_gain_ld_off1 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_gain_ld_off1;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_gain_ld_off2 =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_gain_ld_off2;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_cnt_vsize =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_cnt_vsize;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_big_en =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_big_en;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_nobig_en =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_nobig_en;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_newhist_en =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_newhist_en;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_cnt_mode =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_cnt_mode;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_expl_lgratio =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_expl_lgratio;
        isp_param->ahdr_proc_res.TmoProcRes.sw_hdrtmo_lgscl_ratio =
            ahdr_rk->AhdrProcRes.TmoProcRes.sw_hdrtmo_lgscl_ratio;

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAnr* anr_com =
        mAlogsSharedParams.procResComb.anr_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();
    SmartPtr<rk_aiq_ispp_params_t> ispp_param =
        params->mIsppParams->data();

    if (!anr_com) {
        LOGD_ANALYZER("no anr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen anr common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ANR);
    int algo_id = (*handle)->getAlgoId();

    // gen rk anr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAnrInt* anr_rk = (RkAiqAlgoProcResAnrInt*)anr_com;

#ifdef RK_SIMULATOR_HW
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_anr_proc_res.stBayernrParamSelect,
               &anr_rk->stAnrProcResult.stBayernrParamSelect,
               sizeof(RKAnr_Bayernr_Params_Select_t));
        memcpy(&isp_param->rkaiq_anr_proc_res.stUvnrParamSelect,
               &anr_rk->stAnrProcResult.stUvnrParamSelect,
               sizeof(RKAnr_Uvnr_Params_Select_t));

        memcpy(&isp_param->rkaiq_anr_proc_res.stMfnrParamSelect,
               &anr_rk->stAnrProcResult.stMfnrParamSelect,
               sizeof(RKAnr_Mfnr_Params_Select_t));

        memcpy(&isp_param->rkaiq_anr_proc_res.stYnrParamSelect,
               &anr_rk->stAnrProcResult.stYnrParamSelect,
               sizeof(RKAnr_Ynr_Params_Select_t));

        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rawnr,
               &anr_rk->stAnrProcResult.stBayernrFix,
               sizeof(rk_aiq_isp_rawnr_t));

        ispp_param->update_mask |= RKAIQ_ISPP_NR_ID;
        memcpy(&ispp_param->uvnr,
               &anr_rk->stAnrProcResult.stUvnrFix,
               sizeof(RKAnr_Uvnr_Fix_t));

        memcpy(&ispp_param->ynr,
               &anr_rk->stAnrProcResult.stYnrFix,
               sizeof(RKAnr_Ynr_Fix_t));

        ispp_param->update_mask |= RKAIQ_ISPP_TNR_ID;
        memcpy(&ispp_param->tnr,
               &anr_rk->stAnrProcResult.stMfnrFix,
               sizeof(RKAnr_Mfnr_Fix_t));

        memcpy(&isp_param->gain_config,
               &anr_rk->stAnrProcResult.stGainFix,
               sizeof(rk_aiq_isp_gain_t));

        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);

#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdhazResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAdhaz* adhaz_com =
        mAlogsSharedParams.procResComb.adhaz_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!adhaz_com) {
        LOGD_ANALYZER("no adhaz result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen adhaz common result
    RkAiqAlgoProcResAdhaz* adhaz_rk = (RkAiqAlgoProcResAdhaz*)adhaz_com;

    isp_param->adhaz_config = adhaz_rk->adhaz_config;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADHAZ);
    int algo_id = (*handle)->getAlgoId();

    // gen rk adhaz result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdhazInt* adhaz_rk = (RkAiqAlgoProcResAdhazInt*)adhaz_com;


    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAsdResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAsd* asd_com =
        mAlogsSharedParams.procResComb.asd_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!asd_com) {
        LOGD_ANALYZER("no asd result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asd common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ASD);
    int algo_id = (*handle)->getAlgoId();

    // gen rk asd result
    if (algo_id == 0) {
        RkAiqAlgoProcResAsdInt* asd_rk = (RkAiqAlgoProcResAsdInt*)asd_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAcpResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAcp* acp_com =
        mAlogsSharedParams.procResComb.acp_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!acp_com) {
        LOGD_ANALYZER("no acp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen acp common result
    rk_aiq_acp_params_t* isp_cp = &isp_param->cp;

    *isp_cp = acp_com->acp_res;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACP);
    int algo_id = (*handle)->getAlgoId();

    // gen rk acp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAcpInt* acp_rk = (RkAiqAlgoProcResAcpInt*)acp_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAieResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAie* aie_com =
        mAlogsSharedParams.procResComb.aie_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!aie_com) {
        LOGD_ANALYZER("no aie result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen aie common result
    rk_aiq_isp_ie_t* isp_ie = &isp_param->ie;
    isp_ie->base = aie_com->params;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AIE);
    int algo_id = (*handle)->getAlgoId();

    // gen rk aie result
    if (algo_id == 0) {
        RkAiqAlgoProcResAieInt* aie_rk = (RkAiqAlgoProcResAieInt*)aie_com;

        isp_ie->extra = aie_rk->params;
#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAsharpResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAsharp* asharp_com =
        mAlogsSharedParams.procResComb.asharp_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();
    SmartPtr<rk_aiq_ispp_params_t> ispp_param =
        params->mIsppParams->data();

    if (!asharp_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ASHARP);
    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAsharpInt* asharp_rk = (RkAiqAlgoProcResAsharpInt*)asharp_com;

#ifdef RK_SIMULATOR_HW
        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_asharp_proc_res.stSharpParamSelect.rk_sharpen_params_selected_V1,
               &asharp_rk->stAsharpProcResult.stSharpParamSelect.rk_sharpen_params_selected_V1,
               sizeof(RKAsharp_Sharp_HW_Params_Select_t));

        memcpy(&isp_param->rkaiq_asharp_proc_res.stSharpParamSelect.rk_sharpen_params_selected_V2,
               &asharp_rk->stAsharpProcResult.stSharpParamSelect.rk_sharpen_params_selected_V2,
               sizeof(RKAsharp_Sharp_HW_V2_Params_Select_t));

        memcpy(&isp_param->rkaiq_asharp_proc_res.stSharpParamSelect.rk_sharpen_params_selected_V3,
               &asharp_rk->stAsharpProcResult.stSharpParamSelect.rk_sharpen_params_selected_V3,
               sizeof(RKAsharp_Sharp_HW_V3_Params_Select_t));

        memcpy(&isp_param->rkaiq_asharp_proc_res.stEdgefilterParamSelect,
               &asharp_rk->stAsharpProcResult.stEdgefilterParamSelect,
               sizeof(RKAsharp_EdgeFilter_Params_Select_t));

        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else
        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);

        ispp_param->update_mask |= RKAIQ_ISPP_SHARP_ID;
        memcpy(&ispp_param->sharpen,
               &asharp_rk->stAsharpProcResult.stSharpFix,
               sizeof(rk_aiq_isp_sharpen_t));

        memcpy(&ispp_param->edgeflt,
               &asharp_rk->stAsharpProcResult.stEdgefltFix,
               sizeof(rk_aiq_isp_edgeflt_t));

        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspA3dlutResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResA3dlut* a3dlut_com =
        mAlogsSharedParams.procResComb.a3dlut_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!a3dlut_com) {
        LOGD_ANALYZER("no a3dlut result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen a3dlut common result
    RkAiqAlgoProcResA3dlut* a3dlut_rk = (RkAiqAlgoProcResA3dlut*)a3dlut_com;
    isp_param->lut3d = a3dlut_rk->lut3d_hw_conf;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_A3DLUT);
    int algo_id = (*handle)->getAlgoId();

    // gen rk a3dlut result
    if (algo_id == 0) {
        RkAiqAlgoProcResA3dlutInt* a3dlut_rk_int = (RkAiqAlgoProcResA3dlutInt*)a3dlut_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAblcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAblc* ablc_com =
        mAlogsSharedParams.procResComb.ablc_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!ablc_com) {
        LOGD_ANALYZER("no ablc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ablc common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ABLC);
    int algo_id = (*handle)->getAlgoId();

    // gen rk ablc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAblcInt* ablc_rk = (RkAiqAlgoProcResAblcInt*)ablc_com;

        memcpy(&isp_param->blc, &ablc_rk->ablc_proc_res,
               sizeof(rk_aiq_isp_blc_t));
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAccmResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAccm* accm_com =
        mAlogsSharedParams.procResComb.accm_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!accm_com) {
        LOGD_ANALYZER("no accm result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen accm common result
    RkAiqAlgoProcResAccm* accm_rk = (RkAiqAlgoProcResAccm*)accm_com;
    isp_param->ccm = accm_rk->accm_hw_conf;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACCM);
    int algo_id = (*handle)->getAlgoId();

    // gen rk accm result
    if (algo_id == 0) {
        RkAiqAlgoProcResAccmInt* accm_rk_int = (RkAiqAlgoProcResAccmInt*)accm_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAcgcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAcgc* acgc_com =
        mAlogsSharedParams.procResComb.acgc_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!acgc_com) {
        LOGD_ANALYZER("no acgc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen acgc common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACGC);
    int algo_id = (*handle)->getAlgoId();

    // gen rk acgc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAcgcInt* acgc_rk = (RkAiqAlgoProcResAcgcInt*)acgc_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdebayerResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAdebayer* adebayer_com =
        mAlogsSharedParams.procResComb.adebayer_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!adebayer_com) {
        LOGD_ANALYZER("no adebayer result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen adebayer common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADEBAYER);
    int algo_id = (*handle)->getAlgoId();

    // gen rk adebayer result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdebayerInt* adebayer_rk = (RkAiqAlgoProcResAdebayerInt*)adebayer_com;
        memcpy(&isp_param->demosaic, &adebayer_rk->debayerRes.config, sizeof(AdebayerConfig_t));
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdpccResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAdpcc* adpcc_com =
        mAlogsSharedParams.procResComb.adpcc_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!adpcc_com) {
        LOGD_ANALYZER("no adpcc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen adpcc common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADPCC);
    int algo_id = (*handle)->getAlgoId();

    // gen rk adpcc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdpccInt* adpcc_rk = (RkAiqAlgoProcResAdpccInt*)adpcc_com;

        LOGD_ADPCC("oyyf: %s:%d output dpcc param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->dpcc,
               &adpcc_rk->stAdpccProcResult,
               sizeof(rk_aiq_isp_dpcc_t));
        LOGD_ADPCC("oyyf: %s:%d output dpcc param end\n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAfecResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAfec* afec_com =
        mAlogsSharedParams.procResComb.afec_proc_res;
    SmartPtr<rk_aiq_ispp_params_t> ispp_param =
        params->mIsppParams->data();

    if (!afec_com) {
        LOGD_ANALYZER("no afec result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen afec common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AFEC);
    int algo_id = (*handle)->getAlgoId();

    ispp_param->update_mask |= RKAIQ_ISPP_FEC_ID;
    // gen rk afec result
    if (algo_id == 0) {
        RkAiqAlgoProcResAfecInt* afec_rk = (RkAiqAlgoProcResAfecInt*)afec_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAgammaResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAgamma* agamma_com =
        mAlogsSharedParams.procResComb.agamma_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!agamma_com) {
        LOGD_ANALYZER("no agamma result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen agamma common result
    RkAiqAlgoProcResAgamma* agamma_rk = (RkAiqAlgoProcResAgamma*)agamma_com;

    isp_param->agamma_config = agamma_rk->agamma_config;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AGAMMA);
    int algo_id = (*handle)->getAlgoId();

    // gen rk agamma result
    if (algo_id == 0) {
        RkAiqAlgoProcResAgammaInt* agamma_rk = (RkAiqAlgoProcResAgammaInt*)agamma_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAgicResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAgic* agic_com =
        mAlogsSharedParams.procResComb.agic_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!agic_com) {
        LOGD_ANALYZER("no agic result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen agic common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AGIC);
    int algo_id = (*handle)->getAlgoId();

    // gen rk agic result
    if (algo_id == 0) {
        RkAiqAlgoProcResAgicInt* agic_rk = (RkAiqAlgoProcResAgicInt*)agic_com;
        memcpy(&isp_param->gic, &agic_rk->gicRes.config, sizeof(AgicConfig_t));
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAldchResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAldch* aldch_com =
        mAlogsSharedParams.procResComb.aldch_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!aldch_com) {
        LOGD_ANALYZER("no aldch result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen aldch common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ALDCH);
    int algo_id = (*handle)->getAlgoId();

    // gen rk aldch result
    if (algo_id == 0) {
        RkAiqAlgoProcResAldchInt* aldch_rk = (RkAiqAlgoProcResAldchInt*)aldch_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAlscResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAlsc* alsc_com =
        mAlogsSharedParams.procResComb.alsc_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!alsc_com) {
        LOGD_ANALYZER("no alsc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen alsc common result
    RkAiqAlgoProcResAlsc* alsc_rk = (RkAiqAlgoProcResAlsc*)alsc_com;
    isp_param->lsc = alsc_rk->alsc_hw_conf;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ALSC);
    int algo_id = (*handle)->getAlgoId();

    // gen rk alsc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAlscInt* alsc_rk_int = (RkAiqAlgoProcResAlscInt*)alsc_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAorbResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAorb* aorb_com =
        mAlogsSharedParams.procResComb.aorb_proc_res;
    SmartPtr<rk_aiq_ispp_params_t> ispp_param =
        params->mIsppParams->data();

    if (!aorb_com) {
        LOGD_ANALYZER("no aorb result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen aorb common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AORB);
    int algo_id = (*handle)->getAlgoId();

    ispp_param->update_mask |= RKAIQ_ISPP_ORB_ID;
    // gen rk aorb result
    if (algo_id == 0) {
        RkAiqAlgoProcResAorbInt* aorb_rk = (RkAiqAlgoProcResAorbInt*)aorb_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAr2yResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAr2y* ar2y_com =
        mAlogsSharedParams.procResComb.ar2y_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!ar2y_com) {
        LOGD_ANALYZER("no ar2y result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ar2y common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AR2Y);
    int algo_id = (*handle)->getAlgoId();

    // gen rk ar2y result
    if (algo_id == 0) {
        RkAiqAlgoProcResAr2yInt* ar2y_rk = (RkAiqAlgoProcResAr2yInt*)ar2y_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAwdrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAwdr* awdr_com =
        mAlogsSharedParams.procResComb.awdr_proc_res;
    SmartPtr<rk_aiq_isp_params_t> isp_param =
        params->mIspParams->data();

    if (!awdr_com) {
        LOGD_ANALYZER("no awdr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen awdr common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADEBAYER);
    int algo_id = (*handle)->getAlgoId();

    // gen rk awdr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAwdrInt* awdr_rk = (RkAiqAlgoProcResAwdrInt*)awdr_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::pushStats(SmartPtr<VideoBuffer> &buffer)
{
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(buffer.ptr());
    if (buffer->get_video_info().format == V4L2_META_FMT_RK_ISP1_STAT_3A)
        mRkAiqCoreTh->push_stats(buffer);
    else
        mRkAiqCorePpTh->push_stats(buffer);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

const RkAiqHandle*
RkAiqCore::getAiqAlgoHandle(const int algo_type)
{
    SmartPtr<RkAiqHandle>* handlePtr = getCurAlgoTypeHandle(algo_type);

    return (*handlePtr).ptr();
}

SmartPtr<RkAiqHandle>*
RkAiqCore::getCurAlgoTypeHandle(int algo_type)
{
    SmartPtr<RkAiqHandle>* hdlArray[] = {
        (&mCurAeAlgoHdl),
        (&mCurAwbAlgoHdl),
        (&mCurAfAlgoHdl),
        (&mCurAblcAlgoHdl),
        (&mCurAdpccAlgoHdl),
        (&mCurAhdrAlgoHdl),
        (&mCurAnrAlgoHdl),
        (&mCurAlscAlgoHdl),
        (&mCurAgicAlgoHdl),
        (&mCurAdebayerAlgoHdl),
        (&mCurAccmAlgoHdl),
        (&mCurAgammaAlgoHdl),
        (&mCurAwdrAlgoHdl),
        (&mCurAdhazAlgoHdl),
        (&mCurA3dlutAlgoHdl),
        (&mCurAldchAlgoHdl),
        (&mCurAr2yAlgoHdl),
        (&mCurAcpAlgoHdl),
        (&mCurAieAlgoHdl),
        (&mCurAsharpAlgoHdl),
        (&mCurAorbAlgoHdl),
        (&mCurAfecAlgoHdl),
        (&mCurAcgcAlgoHdl),
        (&mCurAsdAlgoHdl),
    };

    return hdlArray[algo_type];
}

std::map<int, SmartPtr<RkAiqHandle>>*
                                  RkAiqCore::getAlgoTypeHandleMap(int algo_type)
{
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map_array[] = {
        &mAeAlgoHandleMap,
        &mAwbAlgoHandleMap,
        &mAfAlgoHandleMap,
        &mAblcAlgoHandleMap,
        &mAdpccAlgoHandleMap,
        &mAhdrAlgoHandleMap,
        &mAnrAlgoHandleMap,
        &mAlscAlgoHandleMap,
        &mAgicAlgoHandleMap,
        &mAdebayerAlgoHandleMap,
        &mAccmAlgoHandleMap,
        &mAgammaAlgoHandleMap,
        &mAwdrAlgoHandleMap,
        &mAdhazAlgoHandleMap,
        &mA3dlutAlgoHandleMap,
        &mAldchAlgoHandleMap,
        &mAr2yAlgoHandleMap,
        &mAcpAlgoHandleMap,
        &mAieAlgoHandleMap,
        &mAsharpAlgoHandleMap,
        &mAorbAlgoHandleMap,
        &mAfecAlgoHandleMap,
        &mAcgcAlgoHandleMap,
        &mAsdAlgoHandleMap,
    };

    return algo_map_array[algo_type];
}

void
RkAiqCore::addDefaultAlgos()
{
#define ADD_ALGO_HANDLE(lc, BC) \
    mAlogsSharedParams.ctxCfigs[RK_AIQ_ALGO_TYPE_##BC].calib = \
        const_cast<CamCalibDbContext_t*>(mAlogsSharedParams.calib); \
    m##lc##AlgoHandleMap[0] = \
        new RkAiq##lc##HandleInt(g_default_3a_des[RK_AIQ_ALGO_TYPE_##BC], this); \

    ADD_ALGO_HANDLE(Ae, AE);
    ADD_ALGO_HANDLE(Awb, AWB);
    ADD_ALGO_HANDLE(Af, AF);
    ADD_ALGO_HANDLE(Ahdr, AHDR);
    ADD_ALGO_HANDLE(Anr, ANR);
    ADD_ALGO_HANDLE(Adhaz, ADHAZ);
    ADD_ALGO_HANDLE(Asd, ASD);
    ADD_ALGO_HANDLE(Acp, ACP);
    ADD_ALGO_HANDLE(Asharp, ASHARP);
    ADD_ALGO_HANDLE(A3dlut, A3DLUT);
    ADD_ALGO_HANDLE(Ablc, ABLC);
    ADD_ALGO_HANDLE(Accm, ACCM);
    ADD_ALGO_HANDLE(Acgc, ACGC);
    ADD_ALGO_HANDLE(Adebayer, ADEBAYER);
    ADD_ALGO_HANDLE(Adpcc, ADPCC);
    ADD_ALGO_HANDLE(Afec, AFEC);
    ADD_ALGO_HANDLE(Agamma, AGAMMA);
    ADD_ALGO_HANDLE(Agic, AGIC);
    ADD_ALGO_HANDLE(Aie, AIE);
    ADD_ALGO_HANDLE(Aldch, ALDCH);
    ADD_ALGO_HANDLE(Alsc, ALSC);
    ADD_ALGO_HANDLE(Aorb, AORB);
    ADD_ALGO_HANDLE(Ar2y, AR2Y);
    ADD_ALGO_HANDLE(Awdr, AWDR);

#ifdef RK_SIMULATOR_HW
    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++)
        enableAlgo(i, 0, true);
#else
    /*
     * enable the modules that has been verified to work properly on the board
     * TODO: enable all modules after validation in isp
     */
#if 0
    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++)
        enableAlgo(i, 0, true);
#else
    enableAlgo(RK_AIQ_ALGO_TYPE_AE, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_AHDR, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_AWB, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_AGAMMA, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_ABLC, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_ACCM, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_ALSC, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_ADPCC, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_ANR, 0, true);
    /* enableAlgo(RK_AIQ_ALGO_TYPE_AF, 0, true); */
    enableAlgo(RK_AIQ_ALGO_TYPE_ASHARP, 0, true);
    enableAlgo(RK_AIQ_ALGO_TYPE_ADHAZ, 0, true);
    /*enableAlgo(RK_AIQ_ALGO_TYPE_A3DLUT, 0, true); */
#endif
#endif
}

SmartPtr<RkAiqHandle>
RkAiqCore::newAlgoHandle(RkAiqAlgoDesComm* algo)
{
#define NEW_ALGO_HANDLE(lc, BC) \
    if (algo->type == RK_AIQ_ALGO_TYPE_##BC) \
        return new RkAiq##lc##Handle(algo, this);

    NEW_ALGO_HANDLE(Ae, AE);
    NEW_ALGO_HANDLE(Awb, AWB);
    NEW_ALGO_HANDLE(Af, AF);
    NEW_ALGO_HANDLE(Ahdr, AHDR);
    NEW_ALGO_HANDLE(Anr, ANR);
    NEW_ALGO_HANDLE(Adhaz, ADHAZ);
    NEW_ALGO_HANDLE(Asd, ASD);
    NEW_ALGO_HANDLE(Acp, ACP);
    NEW_ALGO_HANDLE(Asharp, ASHARP);
    NEW_ALGO_HANDLE(A3dlut, A3DLUT);
    NEW_ALGO_HANDLE(Ablc, ABLC);
    NEW_ALGO_HANDLE(Accm, ACCM);
    NEW_ALGO_HANDLE(Acgc, ACGC);
    NEW_ALGO_HANDLE(Adebayer, ADEBAYER);
    NEW_ALGO_HANDLE(Adpcc, ADPCC);
    NEW_ALGO_HANDLE(Afec, AFEC);
    NEW_ALGO_HANDLE(Agamma, AGAMMA);
    NEW_ALGO_HANDLE(Agic, AGIC);
    NEW_ALGO_HANDLE(Aie, AIE);
    NEW_ALGO_HANDLE(Aldch, ALDCH);
    NEW_ALGO_HANDLE(Alsc, ALSC);
    NEW_ALGO_HANDLE(Aorb, AORB);
    NEW_ALGO_HANDLE(Ar2y, AR2Y);
    NEW_ALGO_HANDLE(Awdr, AWDR);

    return NULL;
}

XCamReturn
RkAiqCore::addAlgo(RkAiqAlgoDesComm& algo)
{
    ENTER_ANALYZER_FUNCTION();

    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algo.type);

    // TODO, check if exist befor insert ?
    std::map<int, SmartPtr<RkAiqHandle>>::reverse_iterator rit = algo_map->rbegin();

    algo.id = rit->first + 1;

    // add to map
    SmartPtr<RkAiqHandle> new_hdl = newAlgoHandle(&algo);
    new_hdl->setEnable(false);
    (*algo_map)[algo.id] = new_hdl;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::enableAlgo(int algoType, int id, bool enable)
{
    ENTER_ANALYZER_FUNCTION();
    // get current algotype handle, get id
    SmartPtr<RkAiqHandle>* cur_algo_hdl = getCurAlgoTypeHandle(algoType);
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (!cur_algo_hdl) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }

    it->second->setEnable(enable);
    /* WARNING:
     * Be careful when use SmartPtr<RkAiqxxxHandle> = SmartPtr<RkAiqHandle>
     * if RkAiqxxxHandle is derived from multiple RkAiqHandle,
     * the ptr of RkAiqxxxHandle and RkAiqHandle IS NOT the same
     * (RkAiqHandle ptr = RkAiqxxxHandle ptr + offset), but seams like
     * SmartPtr do not deal with this correctly.
     */
    if (enable)
        *cur_algo_hdl = it->second;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::rmAlgo(int algoType, int id)
{
    ENTER_ANALYZER_FUNCTION();

    // can't remove default algos
    if (id == 0)
        return XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* cur_algo_hdl = getCurAlgoTypeHandle(algoType);
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (!cur_algo_hdl) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }

    // if it's the current algo handle, clear it
    if ((*cur_algo_hdl).ptr() == it->second.ptr())
        (*cur_algo_hdl).release();

    algo_map->erase(it);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

bool
RkAiqCore::getAxlibStatus(int algoType, int id)
{
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return false;
    }

    LOGD_ANALYZER("algo type id <%d,%d> status %s", algoType, id,
                  it->second->getEnable() ? "enable" : "disable");

    return it->second->getEnable();
}

const RkAiqAlgoContext*
RkAiqCore::getEnabledAxlibCtx(const int algo_type)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    SmartPtr<RkAiqHandle>* algo_handle = getCurAlgoTypeHandle(algo_type);

    if ((*algo_handle).ptr())
        return (*algo_handle)->getAlgoCtx();
    else
        return NULL;
}

XCamReturn
RkAiqCore::convertIspstatsToAlgo(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef RK_SIMULATOR_HW
    const SmartPtr<Isp20StatsBuffer> buf =
        buffer.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp2x_stat_buffer *stats;

    SmartPtr<RkAiqExpParamsProxy> expParams = buf->get_exp_params();
    SmartPtr<RkAiqIspParamsProxy> ispParams = buf->get_isp_params();
    stats = (struct rkisp_isp2x_stat_buffer *)(buf->get_v4l2_userptr());
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    mAlogsSharedParams.frameId = stats->frame_id;

    //awb2.0
    for(int i = 0; i < RK_AIQ_AWB_MAX_WHITEREGIONS_NUM; i++) {
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Rvalue =
            stats->params.rawawb.ro_rawawb_sum_r_nor[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Gvalue =
            stats->params.rawawb.ro_rawawb_sum_g_nor[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Bvalue =
            stats->params.rawawb.ro_rawawb_sum_b_nor[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_nor[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Rvalue =
            stats->params.rawawb.ro_rawawb_sum_r_big[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Gvalue =
            stats->params.rawawb.ro_rawawb_sum_g_big[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Bvalue =
            stats->params.rawawb.ro_rawawb_sum_b_big[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_big[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Rvalue =
            stats->params.rawawb.ro_rawawb_sum_r_sma[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Gvalue =
            stats->params.rawawb.ro_rawawb_sum_g_sma[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Bvalue =
            stats->params.rawawb.ro_rawawb_sum_b_sma[i];
        mAlogsSharedParams.ispStats.awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_sma[i];
    }
    for(int i = 0; i < RK_AIQ_AWB_MULTIWINDOW_NUM; i++) {
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Rvalue =
            stats->params.rawawb.ro_sum_r_nor_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Gvalue =
            stats->params.rawawb.ro_sum_g_nor_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Bvalue =
            stats->params.rawawb.ro_sum_b_nor_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].WpNo =
            stats->params.rawawb.ro_wp_nm_nor_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Rvalue =
            stats->params.rawawb.ro_sum_r_big_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Gvalue =
            stats->params.rawawb.ro_sum_g_big_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Bvalue =
            stats->params.rawawb.ro_sum_b_big_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].WpNo =
            stats->params.rawawb.ro_wp_nm_big_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Rvalue =
            stats->params.rawawb.ro_sum_r_sma_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Gvalue =
            stats->params.rawawb.ro_sum_g_sma_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Bvalue =
            stats->params.rawawb.ro_sum_b_sma_multiwindow[i];
        mAlogsSharedParams.ispStats.awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].WpNo =
            stats->params.rawawb.ro_wp_nm_sma_multiwindow[i];
    }
    for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V200; i++) {
        mAlogsSharedParams.ispStats.awb_stats.excWpRangeResult[i].Rvalue = stats->params.rawawb.ro_sum_r_exc[i];
        mAlogsSharedParams.ispStats.awb_stats.excWpRangeResult[i].Gvalue = stats->params.rawawb.ro_sum_g_exc[i];
        mAlogsSharedParams.ispStats.awb_stats.excWpRangeResult[i].Bvalue = stats->params.rawawb.ro_sum_b_exc[i];
        mAlogsSharedParams.ispStats.awb_stats.excWpRangeResult[i].WpNo =    stats->params.rawawb.ro_wp_nm_exc[i];

    }
    for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        mAlogsSharedParams.ispStats.awb_stats.blockResult[i].Rvalue = stats->params.rawawb.ramdata[i].r;
        mAlogsSharedParams.ispStats.awb_stats.blockResult[i].Gvalue = stats->params.rawawb.ramdata[i].g;
        mAlogsSharedParams.ispStats.awb_stats.blockResult[i].Bvalue = stats->params.rawawb.ramdata[i].b;
        mAlogsSharedParams.ispStats.awb_stats.blockResult[i].isWP[2] = stats->params.rawawb.ramdata[i].wp & 0x1;
        mAlogsSharedParams.ispStats.awb_stats.blockResult[i].isWP[1] = (stats->params.rawawb.ramdata[i].wp >> 1) & 0x1;
        mAlogsSharedParams.ispStats.awb_stats.blockResult[i].isWP[0] = (stats->params.rawawb.ramdata[i].wp >> 2) & 0x1;
    }
    //mAlogsSharedParams.ispStats.awb_stats_valid = ISP2X_STAT_RAWAWB(stats->meas_type)? true:false;
    mAlogsSharedParams.ispStats.awb_stats_valid = stats->meas_type >> 5 & 1;

    //ahdr
    mAlogsSharedParams.ispStats.ahdr_stats_valid = stats->meas_type >> 16 & 1;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lglow = stats->params.hdrtmo.lglow;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lgmin = stats->params.hdrtmo.lgmin;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lghigh = stats->params.hdrtmo.lghigh;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lgmax = stats->params.hdrtmo.lgmax;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_weightkey = stats->params.hdrtmo.weightkey;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lgmean = stats->params.hdrtmo.lgmean;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lgrange0 = stats->params.hdrtmo.lgrange0;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lgrange1 = stats->params.hdrtmo.lgrange1;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_palpha = stats->params.hdrtmo.palpha;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_lgavgmax = stats->params.hdrtmo.lgavgmax;
    mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_hdrtmo_linecnt = stats->params.hdrtmo.linecnt;
    for(int i = 0; i < 32; i++)
        mAlogsSharedParams.ispStats.ahdr_stats.tmo_stats.ro_array_min_max[i] = stats->params.hdrtmo.min_max[i];

    // TODO: for AEC FPGA test, default setting is hdr-3frame. You can modify working_mode in IQ xml !!
    for(int i = 0; i < ISP2X_YUVAE_MEAN_NUM; i++) {
        mAlogsSharedParams.ispStats.aec_stats.ae_data.yuv.mean[i] = stats->params.yuvae.mean[i];
        if(i < ISP2X_YUVAE_SUBWIN_NUM)
            mAlogsSharedParams.ispStats.aec_stats.ae_data.yuv.ro_yuvae_sumy[i] = stats->params.yuvae.ro_yuvae_sumy[i];
    }

    int FrameNum;
    switch (mAlogsSharedParams.working_mode)
    {
    case RK_AIQ_WORKING_MODE_NORMAL:
        FrameNum = 1;
        break;
    case RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_2_LINE_HDR:
        FrameNum = 2;
        break;
    case RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR:
    case RK_AIQ_ISP_HDR_MODE_3_LINE_HDR:
        FrameNum = 3;
        break;
    default:
        FrameNum = 1;
        break;
    }

    if(FrameNum == 3) {
        mAlogsSharedParams.ispStats.aec_stats_valid = (stats->meas_type >> 11) & (0x01) ? true : false;
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_big.channelr_xy[i] = stats->params.rawae2.data[i].channelr_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_big.channelg_xy[i] = stats->params.rawae2.data[i].channelg_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_big.channelb_xy[i] = stats->params.rawae2.data[i].channelb_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_big.wndx_sumr[i] = stats->params.rawae2.sumr[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_big.wndx_sumg[i] = stats->params.rawae2.sumg[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_big.wndx_sumb[i] = stats->params.rawae2.sumb[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];

            }
        }

        /*
         *         unsigned long chn0_mean = 0, chn1_mean = 0, chn2_mean = 0;
         *         for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
         *             chn0_mean += stats->params.rawae1.data[i].channelg_xy;
         *             chn2_mean += stats->params.rawae2.data[i].channelg_xy;
         *         }
         *
         *         for(int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++)
         *                 chn1_mean += stats->params.rawae0.data[i].channelg_xy;
         *
         *         printf("frame[%d]: chn[0-1-2]_g_mean_xy: %ld-%ld-%ld\n",
         *                 stats->frame_id, chn0_mean/ISP2X_RAWAEBIG_MEAN_NUM,
         *                 chn1_mean/ISP2X_RAWAELITE_MEAN_NUM,
         *                 chn2_mean/ISP2X_RAWAEBIG_MEAN_NUM);
         */


        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawhist_big.bins, stats->params.rawhist2.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
    } else {
        mAlogsSharedParams.ispStats.aec_stats_valid = (stats->meas_type >> 11) & (0x01) ? true : false;
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae2.data[i].channelr_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae2.data[i].channelg_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae2.data[i].channelb_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae2.sumr[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae2.sumg[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae2.sumb[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];

                mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }

        /*
         *         unsigned long chn0_mean = 0, chn1_mean = 0, chn2_mean = 0;
         *         for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
         *             chn0_mean += stats->params.rawae2.data[i].channelg_xy;
         *             chn1_mean += stats->params.rawae1.data[i].channelg_xy;
         *             chn2_mean += stats->params.rawae3.data[i].channelg_xy;
         *         }
         *
         *         printf("frame[%d]: chn[0-1-2]_g_mean_xy: %ld-%ld-%ld\n",
         *                stats->frame_id, chn0_mean/(ISP2X_RAWAEBIG_MEAN_NUM*16),
         *                chn1_mean/(ISP2X_RAWAEBIG_MEAN_NUM*16),
         *                chn2_mean/(ISP2X_RAWAEBIG_MEAN_NUM*16));
         */

        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist2.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.chn[2].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_data.extra.rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
    }

    if (expParams.ptr()) {
        mAlogsSharedParams.ispStats.aec_stats.ae_exp.LinearExp = expParams->data()->LinearExp;
        memcpy(mAlogsSharedParams.ispStats.aec_stats.ae_exp.HdrExp,
               expParams->data()->HdrExp, sizeof(expParams->data()->HdrExp));
        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->data()->HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->data()->HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->data()->HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }


    //af
    {
        mAlogsSharedParams.ispStats.af_stats_valid =
            (stats->meas_type >> 6) & (0x01) ? true : false;
        mAlogsSharedParams.ispStats.af_stats.roia_sharpness =
            stats->params.rawaf.afm_sum[0];
        mAlogsSharedParams.ispStats.af_stats.roia_luminance =
            stats->params.rawaf.afm_lum[0];
        mAlogsSharedParams.ispStats.af_stats.roib_sharpness =
            stats->params.rawaf.afm_sum[1];
        mAlogsSharedParams.ispStats.af_stats.roib_luminance =
            stats->params.rawaf.afm_lum[1];
        memcpy(mAlogsSharedParams.ispStats.af_stats.global_sharpness,
               stats->params.rawaf.ramdata, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
    }
#endif
    return ret;
}

XCamReturn
RkAiqCore::analyze(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqFullParamsProxy> fullParam;
    SmartPtr<RkAiqFullParamsProxy> fullPparam;
    bool has_orb_stats = false;
    bool has_3a_stats = false;

    mAlogsSharedParams.init = false;

#ifdef RK_SIMULATOR_HW
    has_orb_stats = true;
    has_3a_stats = true;
#else
    if (buffer->get_video_info().format == V4L2_META_FMT_RK_ISP1_STAT_3A) {
        has_3a_stats = true;
    } else {
        has_orb_stats = true;
    }
#endif

#ifdef RK_SIMULATOR_HW
    const SmartPtr<V4l2BufferProxy> buf =
        buffer.dynamic_cast_ptr<V4l2BufferProxy>();

    rk_sim_isp_v200_stats_t* stats =
        (rk_sim_isp_v200_stats_t*)(buf->get_v4l2_userptr());
    // copy directly for simulator
    mAlogsSharedParams.ispStats.awb_stats = stats->awb;
    mAlogsSharedParams.ispStats.awb_stats_valid = stats->valid_awb;
    mAlogsSharedParams.ispStats.awb_stats_v201 = stats->awb_v201;

    mAlogsSharedParams.ispStats.aec_stats = stats->ae;
    mAlogsSharedParams.ispStats.aec_stats_valid = stats->valid_ae;

    mAlogsSharedParams.ispStats.af_stats = stats->af;
    mAlogsSharedParams.ispStats.af_stats_valid = stats->valid_af;


#if 1
    LOGD_ANR("oyyf: %s:%d input stats param start\n", __FUNCTION__, __LINE__);
    mAlogsSharedParams.iso = stats->iso;
    LOGD_ANR("oyyf: %s:%d input stats param end\n", __FUNCTION__, __LINE__);
#endif

    //Ahdr
    mAlogsSharedParams.ispStats.ahdr_stats_valid = true;
    mAlogsSharedParams.ispStats.ahdr_stats = stats->ahdr;

    mAlogsSharedParams.ispStats.orb_stats = stats->orb;
    mAlogsSharedParams.ispStats.orb_stats_valid = stats->valid_orb;
#else
    if (has_3a_stats) {
        convertIspstatsToAlgo(buffer);
    } else if (has_orb_stats) {
        // TODO
    } else {
        LOGW_ANALYZER("no orb or 3a stats !", __FUNCTION__, __LINE__);
    }
#endif

    if (has_3a_stats)
        fullParam = analyzeInternal();
    if (has_orb_stats)
        fullPparam = analyzeInternalPp();

#ifdef RK_SIMULATOR_HW
    // merge results for simulator
    fullParam->data()->mIsppParams->data()->orb =
        fullPparam->data()->mIsppParams->data()->orb;
#endif

    if (fullParam.ptr() && mCb) {
        fullParam->data()->mIspParams->data()->frame_id = buffer->get_sequence() + 1;
        mCb->rkAiqCalcDone(fullParam);
    } else if (fullPparam.ptr() && mCb) {
        fullParam->data()->mIsppParams->data()->frame_id = buffer->get_sequence() + 1;
        mCb->rkAiqCalcDone(fullPparam);
    }

    return ret;
}

#define PREPROCESS_ALGO(at) \
    if (mCur##at##AlgoHdl.ptr() && mCur##at##AlgoHdl->getEnable()) { \
        /* TODO, should be called before all algos preProcess ? */ \
        ret = mCur##at##AlgoHdl->updateConfig(); \
        RKAIQCORE_CHECK_BYPASS(ret, "%s updateConfig failed", #at); \
        ret = mCur##at##AlgoHdl->preProcess(); \
        RKAIQCORE_CHECK_BYPASS(ret, "%s preProcess failed", #at); \
    } \

XCamReturn
RkAiqCore::preProcessPp()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // TODO: may adjust the preprocess order

    PREPROCESS_ALGO(Aorb);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::preProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // TODO: may adjust the preprocess order
    PREPROCESS_ALGO(Ae);
    PREPROCESS_ALGO(Awb);
    PREPROCESS_ALGO(Af);
    PREPROCESS_ALGO(Ahdr);
    PREPROCESS_ALGO(Anr);
    PREPROCESS_ALGO(Adhaz);
    PREPROCESS_ALGO(Acp);
    PREPROCESS_ALGO(Asharp);
    PREPROCESS_ALGO(A3dlut);
    PREPROCESS_ALGO(Ablc);
    PREPROCESS_ALGO(Accm);
    PREPROCESS_ALGO(Acgc);
    PREPROCESS_ALGO(Adebayer);
    PREPROCESS_ALGO(Adpcc);
    PREPROCESS_ALGO(Afec);
    PREPROCESS_ALGO(Agamma);
    PREPROCESS_ALGO(Agic);
    PREPROCESS_ALGO(Aie);
    PREPROCESS_ALGO(Aldch);
    PREPROCESS_ALGO(Alsc);
    PREPROCESS_ALGO(Aorb);
    PREPROCESS_ALGO(Ar2y);
    PREPROCESS_ALGO(Awdr);
    PREPROCESS_ALGO(Asd);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

#define PROCESSING_ALGO(at) \
    if (mCur##at##AlgoHdl.ptr() && mCur##at##AlgoHdl->getEnable()) \
        ret = mCur##at##AlgoHdl->processing(); \
    RKAIQCORE_CHECK_BYPASS(ret, "%s processing failed", #at);

XCamReturn
RkAiqCore::processingPp()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    PROCESSING_ALGO(Aorb);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::processing()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // TODO: may adjust the processing order

    PROCESSING_ALGO(Ae);
    PROCESSING_ALGO(Awb);
    PROCESSING_ALGO(Af);
    PROCESSING_ALGO(Ahdr);
    PROCESSING_ALGO(Anr);
    PROCESSING_ALGO(Adhaz);
    PROCESSING_ALGO(Acp);
    PROCESSING_ALGO(Asharp);
    PROCESSING_ALGO(A3dlut);
    PROCESSING_ALGO(Ablc);
    PROCESSING_ALGO(Accm);
    PROCESSING_ALGO(Acgc);
    PROCESSING_ALGO(Adebayer);
    PROCESSING_ALGO(Adpcc);
    PROCESSING_ALGO(Afec);
    PROCESSING_ALGO(Agamma);
    PROCESSING_ALGO(Agic);
    PROCESSING_ALGO(Aie);
    PROCESSING_ALGO(Aldch);
    PROCESSING_ALGO(Alsc);
    PROCESSING_ALGO(Aorb);
    PROCESSING_ALGO(Ar2y);
    PROCESSING_ALGO(Awdr);
    PROCESSING_ALGO(Asd);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

#define POSTPROCESS_ALGO(at) \
    if (mCur##at##AlgoHdl.ptr() && mCur##at##AlgoHdl->getEnable()) \
        ret = mCur##at##AlgoHdl->postProcess(); \
    RKAIQCORE_CHECK_BYPASS(ret, "%s postProcess failed", #at);

XCamReturn
RkAiqCore::postProcessPp()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    POSTPROCESS_ALGO(Aorb);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::postProcess()
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    POSTPROCESS_ALGO(Ae);
    POSTPROCESS_ALGO(Awb);
    POSTPROCESS_ALGO(Af);
    POSTPROCESS_ALGO(Ahdr);
    POSTPROCESS_ALGO(Anr);
    POSTPROCESS_ALGO(Adhaz);
    POSTPROCESS_ALGO(Acp);
    POSTPROCESS_ALGO(Asharp);
    POSTPROCESS_ALGO(A3dlut);
    POSTPROCESS_ALGO(Ablc);
    POSTPROCESS_ALGO(Accm);
    POSTPROCESS_ALGO(Acgc);
    POSTPROCESS_ALGO(Adebayer);
    POSTPROCESS_ALGO(Adpcc);
    POSTPROCESS_ALGO(Afec);
    POSTPROCESS_ALGO(Agamma);
    POSTPROCESS_ALGO(Agic);
    POSTPROCESS_ALGO(Aie);
    POSTPROCESS_ALGO(Aldch);
    POSTPROCESS_ALGO(Alsc);
    POSTPROCESS_ALGO(Aorb);
    POSTPROCESS_ALGO(Ar2y);
    POSTPROCESS_ALGO(Awdr);
    POSTPROCESS_ALGO(Asd);
    // TODO: may adjust the postProcess order
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

} //namespace RkCam

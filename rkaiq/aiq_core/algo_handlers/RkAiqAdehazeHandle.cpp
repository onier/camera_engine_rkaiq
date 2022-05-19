/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#include "RkAiqAdehazeHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdehazeHandleInt);

void RkAiqAdehazeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdhaz());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdhaz());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdhaz());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdehazeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle prepare failed");

    RkAiqAlgoConfigAdhaz* adhaz_config_int      = (RkAiqAlgoConfigAdhaz*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

#ifdef RKAIQ_ENABLE_PARSER_V1
    adhaz_config_int->calib = sharedCom->calib;
#endif

    adhaz_config_int->working_mode      = sharedCom->working_mode;
    adhaz_config_int->is_multi_isp_mode = sharedCom->is_multi_isp_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdehazeHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAdhaz* adhaz_pre_int            = (RkAiqAlgoPreAdhaz*)mPreInParam;
    RkAiqAlgoPreResAdhaz* adhaz_pre_res_int     = (RkAiqAlgoPreResAdhaz*)mPreOutParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    adhaz_pre_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    adhaz_pre_int->rawWidth  = sharedCom->snsDes.isp_acq_width;

    RkAiqIspStats* xIspStats = nullptr;
    if (shared->ispStats) {
        xIspStats = (RkAiqIspStats*)shared->ispStats->map(shared->ispStats);
        if (!xIspStats) LOGE_ADEHAZE("isp stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }

    if (!xIspStats || !xIspStats->adehaze_stats_valid || !sharedCom->init) {
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        LOGE("no adehaze stats, ignore!");
        return XCAM_RETURN_BYPASS;
#endif
    } else {
#if RKAIQ_HAVE_DEHAZE_V10
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v10,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v10,
               sizeof(dehaze_stats_v10_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v11,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v11,
               sizeof(dehaze_stats_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v11_duo,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v11_duo,
               sizeof(dehaze_stats_v11_duo_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v12,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v12,
               sizeof(dehaze_stats_v12_t));
#endif
    }

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle preProcess failed");
    }

#ifdef RK_SIMULATOR_HW
    // nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdehazeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdhaz* adhaz_proc_int        = (RkAiqAlgoProcAdhaz*)mProcInParam;
    RkAiqAlgoProcResAdhaz* adhaz_proc_res_int = (RkAiqAlgoProcResAdhaz*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqIspStats* xIspStats = nullptr;
    if (shared->ispStats) {
        xIspStats = (RkAiqIspStats*)shared->ispStats->map(shared->ispStats);
        if (!xIspStats) LOGE_ADEHAZE("isp stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }

    if (!xIspStats || !xIspStats->adehaze_stats_valid || !sharedCom->init) {
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        LOGE("no adehaze stats, ignore!");
        return XCAM_RETURN_BYPASS;
#endif
    } else {
#if RKAIQ_HAVE_DEHAZE_V10
        memcpy(&adhaz_proc_int->stats.dehaze_stats_v10,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v10,
               sizeof(dehaze_stats_v10_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11
        memcpy(&adhaz_proc_int->stats.dehaze_stats_v11,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v11,
               sizeof(dehaze_stats_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        memcpy(&adhaz_proc_int->stats.dehaze_stats_v11_duo,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v11_duo,
               sizeof(dehaze_stats_v11_duo_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        memcpy(&adhaz_proc_int->stats.dehaze_stats_v12,
               &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v12,
               sizeof(dehaze_stats_v12_t));
#endif
    }
    adhaz_proc_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    adhaz_proc_int->rawWidth  = sharedCom->snsDes.isp_acq_width;

    adhaz_proc_int->hdr_mode = sharedCom->working_mode;

    adhaz_proc_int->ablcV32_proc_res = shared->res_comb.ablcV32_proc_res;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle processing failed");
    }

#ifdef RKAIQ_ENABLE_PARSER_V1
    adhaz_proc_int->pCalibDehaze = sharedCom->calib;
#endif

#ifdef RK_SIMULATOR_HW
    // nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0

    RkAiqAlgoPostAdhaz* adhaz_post_int        = (RkAiqAlgoPostAdhaz*)mPostInParam;
    RkAiqAlgoPostResAdhaz* adhaz_post_res_int = (RkAiqAlgoPostResAdhaz*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_DEHAZE_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_adehaze_v10_SetAttrib(mAlgoCtx, &mCurAttV10, false);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
        mCurAttV11 = mNewAttV11;
        rk_aiq_uapi_adehaze_v11_SetAttrib(mAlgoCtx, &mCurAttV11, false);
        updateAtt = false;
        sendSignal(mCurAttV11.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        mCurAttV12 = mNewAttV12;
        rk_aiq_uapi_adehaze_v12_SetAttrib(mAlgoCtx, &mCurAttV12, false);
        updateAtt = false;
        sendSignal(mCurAttV12.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V10
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV10(const adehaze_sw_v10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV10, att, sizeof(adehaze_sw_v10_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV10, att, sizeof(adehaze_sw_v10_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV10 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV10(adehaze_sw_v10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(updateAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV11(const adehaze_sw_v11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV11, att, sizeof(adehaze_sw_v11_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV11, att, sizeof(adehaze_sw_v11_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV11 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV11(adehaze_sw_v11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v11_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV11, sizeof(updateAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v11_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV11.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV12(const adehaze_sw_v12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV12, att, sizeof(adehaze_sw_v12_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV12, att, sizeof(adehaze_sw_v12_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV12 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV12(adehaze_sw_v12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v12_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12, sizeof(updateAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v12_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
XCamReturn RkAiqAdehazeHandleInt::genIspResult(RkAiqFullParams* params,
                                               RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom  = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdhaz* adhaz_com             = (RkAiqAlgoProcResAdhaz*)mProcOutParam;
    rk_aiq_isp_dehaze_params_v20_t* dehaze_param = params->mDehazeParams->data().ptr();

    if (!adhaz_com) {
        LOGD_ANALYZER("no adhaz result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (sharedCom->init) {
        dehaze_param->frame_id = 0;
    } else {
        dehaze_param->frame_id = shared->frameId;
    }
    dehaze_param->result = adhaz_com->AdehzeProcRes;

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdhaz* adhaz_rk = (RkAiqAlgoProcResAdhaz*)adhaz_com;
    }

    cur_params->mDehazeParams = params->mDehazeParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam

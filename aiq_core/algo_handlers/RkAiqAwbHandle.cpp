/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqCore.h"
#include "RkAiqHandle.h"
#include "RkAiqHandleInt.h"

namespace RkCam {

void RkAiqAwbHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig      = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAwbInt());
    mPreInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPreAwbInt());
    mPreOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAwbInt());
    mProcInParam = (RkAiqAlgoCom*)(new RkAiqAlgoProcAwbInt());
    // mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAwbInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAwbInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAwbInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAwbHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_awb_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }
    if (updateWbV20Attr) {
        mCurWbV20Attr   = mNewWbV20Attr;
        updateWbV20Attr = false;
        rk_aiq_uapiV2_awbV20_SetAttrib(mAlgoCtx, mCurWbV20Attr, false);
        sendSignal();
    }
    if (updateWbOpModeAttr) {
        mCurWbOpModeAttr   = mNewWbOpModeAttr;
        updateWbOpModeAttr = false;
        rk_aiq_uapiV2_awb_SetMwbMode(mAlgoCtx, mCurWbOpModeAttr, false);
        sendSignal();
    }
    if (updateWbMwbAttr) {
        mCurWbMwbAttr   = mNewWbMwbAttr;
        updateWbMwbAttr = false;
        rk_aiq_uapiV2_awb_SetMwbAttrib(mAlgoCtx, mCurWbMwbAttr, false);
        sendSignal();
    }
    if (updateWbAwbAttr) {
        mCurWbAwbAttr   = mNewWbAwbAttr;
        updateWbAwbAttr = false;
        rk_aiq_uapiV2_awbV20_SetAwbAttrib(mAlgoCtx, mCurWbAwbAttr, false);
        sendSignal();
    }
    if (updateWbAwbWbGainAdjustAttr) {
        mCurWbAwbWbGainAdjustAttr   = mNewWbAwbWbGainAdjustAttr;
        updateWbAwbWbGainAdjustAttr = false;
        rk_aiq_uapiV2_awb_SetAwbGainAdjust(mAlgoCtx, mCurWbAwbWbGainAdjustAttr, false);
        sendSignal();
    }
    if (updateWbAwbWbGainOffsetAttr) {
        mCurWbAwbWbGainOffsetAttr   = mNewWbAwbWbGainOffsetAttr;
        updateWbAwbWbGainOffsetAttr = false;
        rk_aiq_uapiV2_awb_SetAwbGainOffset(mAlgoCtx, mCurWbAwbWbGainOffsetAttr, false);
        sendSignal();
    }
    if (updateWbAwbMultiWindowAttr) {
        mCurWbAwbMultiWindowAttr   = mNewWbAwbMultiWindowAttr;
        updateWbAwbMultiWindowAttr = false;
        rk_aiq_uapiV2_awb_SetAwbMultiwindow(mAlgoCtx, mCurWbAwbMultiWindowAttr, false);
        sendSignal();
    }
    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setAttrib(rk_aiq_wb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_wb_attrib_t))) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getAttrib(rk_aiq_wb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_awb_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getCct(rk_aiq_wb_cct_t* cct) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetCCT(mAlgoCtx, cct);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::queryWBInfo(rk_aiq_wb_querry_info_t* wb_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_QueryWBInfo(mAlgoCtx, wb_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::lock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_Lock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::unlock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_Unlock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbV20Attr, &att, sizeof(rk_aiq_uapiV2_wbV20_attrib_t))) {
        mNewWbV20Attr   = att;
        updateWbV20Attr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV20_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbOpModeAttrib(rk_aiq_wb_op_mode_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbOpModeAttr, &att, sizeof(rk_aiq_wb_op_mode_t))) {
        mNewWbOpModeAttr   = att;
        updateWbOpModeAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbOpModeAttrib(rk_aiq_wb_op_mode_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetMwbMode(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setMwbAttrib(rk_aiq_wb_mwb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbMwbAttr, &att, sizeof(rk_aiq_wb_mwb_attrib_t))) {
        mNewWbMwbAttr   = att;
        updateWbMwbAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getMwbAttrib(rk_aiq_wb_mwb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetMwbAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbAttr, &att, sizeof(rk_aiq_uapiV2_wbV20_awb_attrib_t))) {
        mNewWbAwbAttr   = att;
        updateWbAwbAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV20_GetAwbAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 !=
        memcmp(&mCurWbAwbWbGainAdjustAttr, &att, sizeof(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t))) {
        mNewWbAwbWbGainAdjustAttr   = att;
        updateWbAwbWbGainAdjustAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAwbGainAdjust(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbWbGainOffsetAttrib(CalibDbV2_Awb_gain_offset_cfg_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbWbGainOffsetAttr, &att, sizeof(CalibDbV2_Awb_gain_offset_cfg_t))) {
        mNewWbAwbWbGainOffsetAttr   = att;
        updateWbAwbWbGainOffsetAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbWbGainOffsetAttrib(CalibDbV2_Awb_gain_offset_cfg_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAwbGainOffset(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbMultiWindowAttrib(CalibDbV2_Awb_Mul_Win_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbMultiWindowAttr, &att, sizeof(CalibDbV2_Awb_Mul_Win_t))) {
        mNewWbAwbMultiWindowAttr   = att;
        updateWbAwbMultiWindowAttr = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbMultiWindowAttrib(CalibDbV2_Awb_Mul_Win_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAwbMultiwindow(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "awb handle prepare failed");

    RkAiqAlgoConfigAwbInt* awb_config_int = (RkAiqAlgoConfigAwbInt*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    // TODO
    // awb_config_int->rawBit;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "awb algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAwbInt* awb_pre_int        = (RkAiqAlgoPreAwbInt*)mPreInParam;
    RkAiqAlgoPreResAwbInt* awb_pre_res_int = (RkAiqAlgoPreResAwbInt*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPreResComb* comb                       = &shared->preResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats                     = shared->ispStats;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        comb->awb_pre_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awb handle preProcess failed");
    }
    comb->awb_pre_res     = NULL;

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    int module_hw_version = sharedCom->ctxCfigs[RK_AIQ_ALGO_TYPE_AWB].cfg_com.module_hw_version;
#ifdef RK_SIMULATOR_HW
    if (module_hw_version == AWB_HARDWARE_V200) {
        awb_pre_int->awb_hw0_statis      = ispStats->awb_stats;
        awb_pre_int->awb_cfg_effect_v200 = ispStats->awb_cfg_effect_v200;
    } else {
        awb_pre_int->awb_hw1_statis      = ispStats->awb_stats_v201;
        awb_pre_int->awb_cfg_effect_v201 = ispStats->awb_cfg_effect_v201;
    }
#else
    awb_pre_int->awbStatsBuf = shared->awbStatsBuf;
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "awb algo pre_process failed");
    // set result to mAiqCore
    comb->awb_pre_res = (RkAiqAlgoPreResAwb*)awb_pre_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAwbInt* awb_proc_int = (RkAiqAlgoProcAwbInt*)mProcInParam;
#if 0
    RkAiqAlgoProcResAwbInt* awb_proc_res_int = (RkAiqAlgoProcResAwbInt*)mProcOutParam;
#else
    mProcResShared           = new RkAiqAlgoProcResAwbIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new awb mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAwbInt* awb_proc_res_int = &mProcResShared->result;
    // mProcOutParam = (RkAiqAlgoResCom*)awb_proc_res_int;
#endif
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqProcResComb* comb                      = &shared->procResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats                     = shared->ispStats;

    ret = RkAiqHandle::processing();
    if (ret) {
        comb->awb_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awb handle processing failed");
    }

    comb->awb_proc_res = NULL;

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
#if 0
    ret = des->processing(mProcInParam, mProcOutParam);
#else
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)awb_proc_res_int);
#endif
    RKAIQCORE_CHECK_RET(ret, "awb algo processing failed");
    // set result to mAiqCore
    comb->awb_proc_res = (RkAiqAlgoProcResAwb*)awb_proc_res_int;

    mProcResShared->set_sequence(shared->frameId);
    SmartPtr<XCamMessage> msg =
        new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AWB_PROC_RES_OK, shared->frameId, mProcResShared);
    mAiqCore->post_message(msg);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAwbInt* awb_post_int        = (RkAiqAlgoPostAwbInt*)mPostInParam;
    RkAiqAlgoPostResAwbInt* awb_post_res_int = (RkAiqAlgoPostResAwbInt*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqPostResComb* comb                      = &shared->postResComb;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqIspStats* ispStats                     = shared->ispStats;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        comb->awb_post_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "awb handle postProcess failed");
        return ret;
    }

    comb->awb_post_res        = NULL;
    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "awb algo post_process failed");
    // set result to mAiqCore
    comb->awb_post_res = (RkAiqAlgoPostResAwb*)awb_post_res_int;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAwb* awb_com                = shared->procResComb.awb_proc_res;

    if (!awb_com) {
        LOGD_ANALYZER("no awb result");
        return XCAM_RETURN_NO_ERROR;
    }

#if defined(ISP_HW_V30)
    rk_aiq_isp_awb_params_v3x_t* awb_param = params->mAwbV3xParams->data().ptr();
#elif defined(ISP_HW_V21)
    rk_aiq_isp_awb_params_v21_t* awb_param = params->mAwbV21Params->data().ptr();
#else
    rk_aiq_isp_awb_params_v20_t* awb_param = params->mAwbParams->data().ptr();
#endif
    rk_aiq_isp_awb_gain_params_v20_t* awb_gain_param = params->mAwbGainParams->data().ptr();
    RkAiqAlgoProcResAwb* awb_rk                      = (RkAiqAlgoProcResAwb*)awb_com;

#if 0
    isp_param->awb_gain_update = awb_rk->awb_gain_update;
    isp_param->awb_cfg_update = awb_rk->awb_cfg_update;
    isp_param->awb_gain = awb_rk->awb_gain_algo;
    isp_param->awb_cfg = awb_rk->awb_hw0_para;
    //isp_param->awb_cfg_v201 = awb_rk->awb_hw1_para;
#else
    // TODO: update states
    // awb_gain_param->result.awb_gain_update = awb_rk->awb_gain_update;
    // isp_param->awb_cfg_update = awb_rk->awb_cfg_update;
    if (sharedCom->init) {
        awb_gain_param->frame_id = 0;
        awb_param->frame_id      = 0;
    } else {
        awb_gain_param->frame_id = shared->frameId;
        awb_param->frame_id      = shared->frameId;
    }

    awb_gain_param->result     = awb_rk->awb_gain_algo;
#if defined(ISP_HW_V30) || defined(ISP_HW_V21)
    awb_param->result = awb_rk->awb_hw1_para;
#else
    awb_param->result = awb_rk->awb_hw0_para;
#endif

#endif

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAwbInt* awb_rk_int = (RkAiqAlgoProcResAwbInt*)awb_com;
    }

#if defined(ISP_HW_V30)
    cur_params->mAwbV3xParams  = params->mAwbV3xParams;
#elif defined(ISP_HW_V21)
    cur_params->mAwbV21Params  = params->mAwbV21Params;
#else
    cur_params->mAwbParams     = params->mAwbParams;
#endif
    cur_params->mAwbGainParams = params->mAwbGainParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam

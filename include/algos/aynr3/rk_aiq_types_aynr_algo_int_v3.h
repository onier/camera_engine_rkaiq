/*
 *rk_aiq_types_alsc_algo_int.h
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

#ifndef _RK_AIQ_TYPE_AYNR_ALGO_INT_V3_H_
#define _RK_AIQ_TYPE_AYNR_ALGO_INT_V3_H_

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "aynr3/rk_aiq_types_aynr_algo_v3.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "ynr_head_v3.h"


//RKAIQ_BEGIN_DECLARE
#define RK_YNR_V3_MAX_ISO_NUM (CALIBDB_MAX_ISO_LEVEL)


#define AYNRV3_RECALCULATE_DELTA_ISO       (10)
#define YNR_V3_ISO_CURVE_POINT_BIT          4
#define YNR_V3_ISO_CURVE_POINT_NUM          ((1 << YNR_V3_ISO_CURVE_POINT_BIT)+1)
#define YNR_V3_SIGMA_BITS                  10
#define YNR_V3_NOISE_SIGMA_FIX_BIT              3
#define LOG2(x)                             (log((double)x)                 / log((double)2))


typedef enum Aynr_result_V3_e {
    AYNRV3_RET_SUCCESS             = 0,   // this has to be 0, if clauses rely on it
    AYNRV3_RET_FAILURE             = 1,   // process failure
    AYNRV3_RET_INVALID_PARM        = 2,   // invalid parameter
    AYNRV3_RET_WRONG_CONFIG        = 3,   // feature not supported
    AYNRV3_RET_BUSY                = 4,   // there's already something going on...
    AYNRV3_RET_CANCELED            = 5,   // operation canceled
    AYNRV3_RET_OUTOFMEM            = 6,   // out of memory
    AYNRV3_RET_OUTOFRANGE          = 7,   // parameter/value out of range
    AYNRV3_RET_NULL_POINTER        = 8,   // the/one/all parameter(s) is a(are) NULL pointer(s)
    AYNRV3_RET_DIVISION_BY_ZERO    = 9,   // a divisor equals ZERO
    AYNRV3_RET_NO_INPUTIMAGE       = 10   // no input image
} Aynr_result_V3_t;

typedef enum Aynr_State_V3_e {
    AYNRV3_STATE_INVALID           = 0,                   /**< initialization value */
    AYNRV3_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
    AYNRV3_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
    AYNRV3_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
    AYNRV3_STATE_LOCKED            = 4,                   /**< instance is locked (for taking snapshots) */
    AYNRV3_STATE_MAX                                      /**< max */
} Aynr_State_V3_t;

typedef enum Aynr_OPMode_V3_e {
    AYNRV3_OP_MODE_INVALID           = 0,                   /**< initialization value */
    AYNRV3_OP_MODE_AUTO              = 1,                   /**< instance is created, but not initialized */
    AYNRV3_OP_MODE_MANUAL            = 2,                   /**< instance is confiured (ready to start) or stopped */
    AYNRV3_OP_MODE_MAX                                      /**< max */
} Aynr_OPMode_V3_t;

typedef enum Aynr_ParamMode_V3_e {
    AYNRV3_PARAM_MODE_INVALID           = 0,
    AYNRV3_PARAM_MODE_NORMAL          = 1,                   /**< initialization value */
    AYNRV3_PARAM_MODE_HDR              = 2,                   /**< instance is created, but not initialized */
    AYNRV3_PARAM_MODE_GRAY            = 3,                   /**< instance is confiured (ready to start) or stopped */
    AYNRV3_PARAM_MODE_MAX                                      /**< max */
} Aynr_ParamMode_V3_t;

typedef struct Aynr_ExpInfo_V3_s {
    int hdr_mode;
    float arTime[3];
    float arAGain[3];
    float arDGain[3];
    int   arIso[3];
    int   snr_mode;
    int rawWidth;
    int rawHeight;
} Aynr_ExpInfo_V3_t;

typedef struct RK_YNR_Params_V3_Select_s
{
    int enable;
    float ciISO_V3[2];
    float noiseSigma_V3[YNR_V3_ISO_CURVE_POINT_NUM];
    short lumaPoints_V3[YNR_V3_ISO_CURVE_POINT_NUM];

    float loFreqLumaNrCurvePoint[6];
    float loFreqLumaNrCurveRatio[6];

    float hiFreqLumaNrCurvePoint[6];
    float hiFreqLumaNrCurveRatio[6];

    // low frequency
    int ynr_big_resolution_mode;
    float ynr_rnr_strength_V3[17];
    int ynr_bft3x3_bypass_V3;
    int ynr_lbft5x5_bypass_V3;
    int ynr_lgft3x3_bypass_V3;
    int ynr_flt1x1_bypass_V3;
    int ynr_sft5x5_bypass_V3;
    float ynr_low_bf_V3[2];
    float ynr_low_thred_adj_V3;
    float ynr_low_peak_supress_V3;
    float ynr_low_edge_adj_thresh_V3;
    float ynr_low_bi2_weight_thresh_V3;
    float ynr_low_center_weight_V3;
    float ynr_low_dist_adj_V3;
    float ynr_low_weight_V3;
    float ynr_low_filt1_strength_V3;
    float ynr_low_filt2_strength_V3;
    float ynr_low_bi_weight_V3;

    // high frequency
    float ynr_base_filter_weight1_V3;
    float ynr_base_filter_weight2_V3;
    float ynr_base_filter_weight3_V3;
    float ynr_high_thred_adj_V3;
    float ynr_high_weight_V3;
    float ynr_direction_weight_V3[8];
    float ynr_hi_min_adj_V3;
    float ynr_hi_edge_thed_V3;

    //local gain control
    float ynr_global_gain_alpha_V3;
    float ynr_global_gain_V3;
    float ynr_gain_adjust_thresh_V3;
    float ynr_gain_adjust_scale_V3;
} RK_YNR_Params_V3_Select_t;



typedef struct RK_YNR_Params_V3_s
{
    int enable;
    char version[64];
    float iso[RK_YNR_V3_MAX_ISO_NUM];
    RK_YNR_Params_V3_Select_t arYnrParamsISO[RK_YNR_V3_MAX_ISO_NUM];

} RK_YNR_Params_V3_t;


typedef struct Aynr_Manual_Attr_V3_s
{
    int ynrEn;
    RK_YNR_Params_V3_Select_t stSelect;

} Aynr_Manual_Attr_V3_t;

typedef struct Aynr_Auto_Attr_V3_s
{
    //all ISO params and select param
    int ynrEn;

    RK_YNR_Params_V3_t stParams;
    RK_YNR_Params_V3_Select_t stSelect;

} Aynr_Auto_Attr_V3_t;

typedef struct Aynr_ProcResult_V3_s {
    int ynrEn;

    //for sw simultaion
    RK_YNR_Params_V3_Select_t stSelect;

    //for hw register
    RK_YNR_Fix_V3_t stFix;

    bool isNeedUpdate;

} Aynr_ProcResult_V3_t;


typedef struct Aynr_Config_V3_s {
    Aynr_State_V3_t eState;
    Aynr_OPMode_V3_t eMode;
    int rawHeight;
    int rawWidth;
} Aynr_Config_V3_t;


typedef struct rk_aiq_ynr_attrib_v3_s {
    Aynr_OPMode_V3_t eMode;
    Aynr_Auto_Attr_V3_t stAuto;
    Aynr_Manual_Attr_V3_t stManual;
} rk_aiq_ynr_attrib_v3_t;


typedef struct rk_aiq_ynr_IQPara_V3_s {
    struct list_head* listHead;
} rk_aiq_ynr_IQPara_V3_t;


//calibdb


//RKAIQ_END_DECLARE

#endif


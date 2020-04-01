/*
* rk_aiq_types_ae_algo_int.h

* for rockchip v2.0.0
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
/* for rockchip v2.0.0*/

#ifndef __RK_AIQ_TYPES_AE_ALGO_INT_H__
#define __RK_AIQ_TYPES_AE_ALGO_INT_H__

/**
 * @file rk_aiq_types_ae_algo_int.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup AEC Auto exposure control Module
 * @{
 *
 */

#include "RkAiqCalibDbTypes.h"
#include "rk_aiq_types_ae_algo.h"
#include "awb/rk_aiq_types_awb_algo_int.h"


#define MAX_HDR_FRAMENUM  (3)
#define AEC_AFPS_MASK (1 << 0)
#define AEC_EFFECT_FNUM 3
#define HDR_AEC_EFFECT_FNUM 5
#define MAX_AEC_EFFECT_FNUM ((AEC_EFFECT_FNUM > HDR_AEC_EFFECT_FNUM) ? AEC_EFFECT_FNUM : HDR_AEC_EFFECT_FNUM)
#define ZLJ_DEBUG

/*****************************************************************************/
/**
 * @brief   The number of mean and hist.
 */
/*****************************************************************************/
/*zlj add*/
#define ISP2_RAWAE_LOGHISTBIN_NUM 100
#define ISP2_RAWAE_LITE_GRIDNUM 25
#define ISP2_RAWAE_BIG_GRIDNUM (15*15)
#define ISP2_RAWAE_BIG_WINX_NUM 4

#define ISP2_RAWHIST_BINNUM_MAX  (1<<8)
#define ISP2_RAWAE_GRIDNUM_MAX  ((ISP2_RAWAE_LITE_GRIDNUM > ISP2_RAWAE_BIG_GRIDNUM) ? ISP2_RAWAE_LITE_GRIDNUM : ISP2_RAWAE_BIG_GRIDNUM)

/*****************************************************************************/
/**
 * @brief   CamerIcIspStats related params
 */
/*****************************************************************************/
typedef enum CamIcYRangeMode_e {
    CAMIC_YRANGE_MODE_INVALID    = 0,    /**< invalid y range mode   */
    CAMIC_YRANGE_MODE_FULL       = 1,    /**< Y = 0.299 * R + 0.587 * G + 0.114 * B,full range BT601*/
    CAMIC_YRANGE_MODE_LIMITED    = 2,    /**< Y = 16 + 0.25R + 0.5G + 0.1094B,limited range*/
    CAMIC_YRANGE_MODE_MAX,
} CamIcYRangeMode_t;

typedef enum CamIcRawStatsMode_e {
    CAMIC_RAWSTATS_MODE_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
    CAMIC_RAWSTATS_MODE_R             = 1,    /**< R channel */
    CAMIC_RAWSTATS_MODE_G             = 2,    /**< G channel */
    CAMIC_RAWSTATS_MODE_B             = 3,    /**< B channel */
    CAMIC_RAWSTATS_MODE_Y             = 4,    /**< luminance channel */
    CAMIC_RAWSTATS_MODE_MAX,                                  /**< upper border (only for an internal evaluation) */
} CamIcRawStatsMode_t;

typedef enum CamIcHistStatsMode_e {
    CAMIC_HIST_MODE_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
    CAMIC_HIST_MODE_RGB_COMBINED  = 1,    /**< RGB combined histogram */
    CAMIC_HIST_MODE_R             = 2,    /**< R histogram */
    CAMIC_HIST_MODE_G             = 3,    /**< G histogram */
    CAMIC_HIST_MODE_B             = 4,    /**< B histogram */
    CAMIC_HIST_MODE_Y             = 5,    /**< luminance histogram */
    CAMIC_HIST_MODE_MAX,                                  /**< upper border (only for an internal evaluation) */
} CamIcHistStatsMode_t;

/*****************************************************************************/
/**
 *          CamerIcRawSwapMode_t
 *
 * @brief   mode type of RawSwap
 * 3frame:
 *             L-BIG   M-BIG     S-LITE
 *             L-BIG   M-LITE   S-BIG
 *             L-LITE  M-BIG    S-BIG
 * for Hdr 3frame all 3 modes is effective
 * for Hdr 2frame all 3 modes is effective
 * for Linear mode2 is the same as mode3, mode 1=LITE & mode 2/3=BIG
 */
/*****************************************************************************/
typedef enum CamerIcRawSwapMode_e {
    AEC_RAWSWAP_MODE_S_LITE    = 0,
    AEC_RAWSWAP_MODE_M_LITE    = 1,
    AEC_RAWSWAP_MODE_L_LITE    = 2,
    AEC_RAWSWAP_MODE_MAX,
} CamerIcRawSwapMode_t;

/*****************************************************************************/
/**
 *          AIrisFNO_e
 *
 * @brief   enum Auto Iris FNO
 *          real Equivalent Iris Gain = 1 << AIRIS_F_NO_XX_X
 */
/*****************************************************************************/
typedef enum {
    AIRIS_F_NO_32_0 = 0,
    AIRIS_F_NO_22_0,
    AIRIS_F_NO_16_0,
    AIRIS_F_NO_11_0,
    AIRIS_F_NO_8_0,
    AIRIS_F_NO_5_6,
    AIRIS_F_NO_4_0,
    AIRIS_F_NO_2_8,
    AIRIS_F_NO_2_0,
    AIRIS_F_NO_1_4,
    AIRIS_F_NO_1_0
} AIrisFNO_t;
/*****************************************************************************/
/**
 *          AecDampingMode_t
 *
 * @brief   mode type of AEC Damping
 *
 */
/*****************************************************************************/
typedef enum {
    AEC_DAMPING_MODE_INVALID        = 0,        /* invalid (only used for initialization) */
    AEC_DAMPING_MODE_STILL_IMAGE    = 1,        /* damping mode still image */
    AEC_DAMPING_MODE_VIDEO          = 2,        /* damping mode video */
    AEC_DAMPING_MODE_MAX
} AecDampingMode_t;
/*****************************************************************************/
/**
 * @brief   mode type of Hdr-AEC
 *
 ****************************************************************************/
typedef enum {
    AEC_WORKING_MODE_NORMAL,
    AEC_WORKING_MODE_ISP_HDR2    = 0x10,
    AEC_WORKING_MODE_ISP_HDR3    = 0x20,
} AecWorkingMode_t;

typedef enum {
    ISP_HDR_MODE_2_FRAME_HDR = AEC_WORKING_MODE_ISP_HDR2 + 1,
    ISP_HDR_MODE_2_LINE_HDR = AEC_WORKING_MODE_ISP_HDR2 + 2,
    ISP_HDR_MODE_3_FRAME_HDR = AEC_WORKING_MODE_ISP_HDR3 + 1,
    ISP_HDR_MODE_3_LINE_HDR = AEC_WORKING_MODE_ISP_HDR3 + 2,
} IspHdrMode_t;

typedef enum {
    AEC_LINEHDR_MODE_DCG       = 0,        /* HDR DCG MODE (only one integration time value) */
    AEC_LINEHDR_MODE_STAGGER   = 1,     /* HDR STAGGER MODE (independent integration time value) */
} AecLineHdrMode_t;

/*****************************************************************************/
/**
 *          AecEcmMode_t
 *
 * @brief   mode type of AEC Exposure Conversion
 *
 */
/*****************************************************************************/
typedef enum {
    AEC_EXPOSURE_CONVERSION_INVALID = 0,        /* invalid (only used for initialization) */
    AEC_EXPOSURE_CONVERSION_LINEAR  = 1,         /* Exposure Conversion uses a linear function (eq. 38) */
    AEC_EXPOSURE_CONVERSION_MAX
} AecEcmMode_t;

/*****************************************************************************/
/**
 *          AecFrameMode_t
 * @brief   Frame Mode for RawAe
 *
 * For LinearAe CH0=Normal Frame, CH1/2 not used
 * For HdrAe 2frame CH0=S Frame, CH1=L Frame, CH2 not used
 * For HdrAe 3frame CH0=S Frame, CH1=M Frame, CH2=L Frame
 */
/*****************************************************************************/
typedef enum {
    AEC_HDR_3FRAME_S = 0,
    AEC_HDR_3FRAME_M = 1,
    AEC_HDR_3FRAME_L = 2,

    AEC_HDR_2FRAME_S = 0,
    AEC_HDR_2FRAME_L = 1,

    AEC_NORMAL_FRAME = 0
} AecFrameMode_t;

/*****************************************************************************/
/**
 *          AecProcStrategyMode_t
 *
 * @brief   flicker period types for the AEC algorithm
 *
 */
/*****************************************************************************/
typedef enum {
    LOWLIGHT_DETECT = 0,
    HIGHLIGHT_DETECT = 1
} AecProcStrategyMode_t;

/*****************************************************************************/
/**
 *          AecEcmFlickerPeriod_t
 *
 * @brief   flicker period types for the AEC algorithm
 *
 */
/*****************************************************************************/
typedef enum {
    AEC_EXPOSURE_CONVERSION_FLICKER_OFF   = 0x00,
    AEC_EXPOSURE_CONVERSION_FLICKER_100HZ = 0x01,
    AEC_EXPOSURE_CONVERSION_FLICKER_120HZ = 0x02
} AecEcmFlickerPeriod_t;

typedef enum
{
    TRIGGER_OFF = 0, //no trigger DayNight Switch, use fixed DNMode!
    HARDWARE_TRIGGER = 1, //have photosensitive element to trigger night mode  (hardware Trigger)
    SOFTWARE_TRIGGER = 2, //use software params to trigger night mode  (software Trigger)
} AecNightTriggerMode_t;

typedef enum
{
    IR_BW = 0, //use IR LED as supplement light, black&white night-vision mode (software)
    VB_RGB = 1, //use (visible light/no light) as supplement light, rgb night mode (software)
} AecNightMode_t;

typedef struct Aec_daynight_th_s {
    float         fac_th;
    uint8_t       holdon_times_th;
} Aec_daynight_th_t;

/*****************************************************************************/
/**
 *          Aec_stat_t
 *
 * @brief   AEC Module Hardware statistics structure
 *
 *****************************************************************************/

typedef RkAiqAecHwStatsRes_t AecStat_t;
typedef RKAiqAecExpInfo_t ExpInfo_t;

/*****************************************************************************/
/**
 *          AecConfig_t
 *
 * @brief   AEC Module configuration structure isp2; used for re-configuration as well
 *
 *****************************************************************************/
typedef struct AecConfig_s {

    /*Aec Ctrl Configuration from calibdb, support User Api input Ctrl configuration*/
    CalibDb_AecCommon_Attr_t      CommCtrl;
    CalibDb_LinearAE_Attr_t       LinearAeCtrl;
    CalibDb_HdrAE_Attr_t          HdrAeCtrl;
    RkAiqAecHwConfig_t            HwCtrl;
    CalibDb_Sensor_Para_t         SensorInfo;

    int                           Workingmode;
    int                           LineHdrMode;

    /*params related to driver setting*/
    float                         LinePeriodsPerField;
    float                         PixelClockFreqMHZ;
    float                         PixelPeriodsPerLine;

    /*continue to use some old params to keep the same with AecConfig_t*/
    AecDampingMode_t              DampingMode;              /**< damping mode */
    AecEcmFlickerPeriod_t         EcmFlickerSelect;        /**< flicker period selection */

    int                           RawWidth;
    int                           RawHeight;
    /*api update attr flag*/
    bool                          ApiReconfig;
} AecConfig_t;
/*****************************************************************************/
/**
 *          AeInstanceConfig_t
 *
 * @brief   AF Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AecContext_s* AeHandle_t;     /**< handle to AEC context */
typedef struct AecConfig_s* AeConfig_t;     /**< handle to AEC config */

typedef struct AeInstanceConfig_s {
    AeHandle_t              hAe;            /**< handle returned by AeInit() */
    AeConfig_t              aecCfg;
} AeInstanceConfig_t;

/*****************************************************************************/
/**
 * @brief   ISP2.0 AEC Algo Result Params
 */
/*****************************************************************************/
typedef struct AecPreResult_s {
    float MeanLuma;
    float DarkROILuma;
    float DarkROIPdf;
    float MaxLuma;
    float MaxLumaROIPdf;
    float L2M_ExpRatio;
    float M2S_ExpRatio;

    float GlobalEnvLv;
    float BlockEnvLv[ISP2_RAWAE_BIG_GRIDNUM];
    float DynamicRange;

    unsigned char NormalIndex;

    RkAiqExpParamComb_t LinearExp;
    RkAiqExpParamComb_t HdrExp[MAX_HDR_FRAMENUM];
} AecPreResult_t;

/*****************************************************************************/
/**
*     AecResult_t
* @brief   Aec_Result.
*
*/
/*****************************************************************************/
typedef struct AecProcResult_s {
    /***Common results****/
    RKAiqAecExpInfo_t             cur_ae_exp;
    RKAiqAecExpInfo_t             new_ae_exp;
    rk_aiq_ae_meas_params_t       ae_meas;
    rk_aiq_hist_meas_params_t     hist_meas;

    bool                          IsHdr;
    bool                          IsConverged;
    unsigned int                  actives;
    bool                          auto_adjust_fps;

    /*AE interpolation results to make ae more smooth*/
    RkAiqExpParamComb_t     InterpLinAe[AEC_EFFECT_FNUM];
    RkAiqExpParamComb_t     InterpHdrAe[HDR_AEC_EFFECT_FNUM][MAX_HDR_FRAMENUM];

    RkAiqExpParamComb_t     exp_set[MAX_AEC_EFFECT_FNUM];
    int                     exp_set_cnt;

    /***LinearAe results****/
    float                   InputExposure;
    float                   Exposure;

    float                   MinGain;
    float                   MaxGain;
    float                   MaxGainRange;
    float                   MinIntegrationTime;
    float                   MaxIntegrationTime;

    uint8_t                 GridWeights[ISP2_RAWAE_GRIDNUM_MAX];//used for histogram calculation

    float                   MeanLuma;
    float                   LumaDeviation;

    bool_t                  aoe_enable;
    CalibDb_AecDayNightMode_t  DNMode;
    float                   DON_Fac;
    uint8_t                 DNTrigger;
    uint8_t                 NightMode;
    float                   overHistPercent;

    /***Hdr results****/
    float                   HdrMeanLuma[3];
    float                   HdrLumaDeviation[MAX_HDR_FRAMENUM];

    float                   HdrInputExposure[MAX_HDR_FRAMENUM];
    float                   HdrExposure[MAX_HDR_FRAMENUM];
    float                   HdrMinGain[MAX_HDR_FRAMENUM];
    float                   HdrMaxGain[MAX_HDR_FRAMENUM];
    float                   HdrMinIntegrationTime[MAX_HDR_FRAMENUM];
    float                   HdrMaxIntegrationTime[MAX_HDR_FRAMENUM];

    uint8_t                 HdrGridWeights[3][ISP2_RAWAE_GRIDNUM_MAX];//used for histogram calculation
} AecProcResult_t;

/* @} AEC */

#endif /* __RK_AIQ_TYPES_AE_ALGO_INT_H__*/

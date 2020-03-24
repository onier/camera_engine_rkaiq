/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_TYPES_AHDR_ALGO_INT_H__
#define __RK_AIQ_TYPES_AHDR_ALGO_INT_H__

typedef struct MergeIOData_s
{
    unsigned char  sw_hdrmge_mode;
    unsigned short sw_hdrmge_gain0_inv;
    unsigned short sw_hdrmge_gain0;
    unsigned short sw_hdrmge_gain1_inv;
    unsigned short sw_hdrmge_gain1;
    unsigned char  sw_hdrmge_gain2;
    unsigned char  sw_hdrmge_ms_dif_0p8;
    unsigned char  sw_hdrmge_lm_dif_0p9;
    unsigned char  sw_hdrmge_ms_dif_0p15;
    unsigned char  sw_hdrmge_lm_dif_0p15;
    unsigned short sw_hdrmge_l0_y[17];
    unsigned short sw_hdrmge_l1_y[17];
    unsigned short sw_hdrmge_e_y[17];
} MergeIOData_t;


typedef struct TmoIOData_s
{
    unsigned short sw_hdrtmo_cnt_vsize;
    unsigned char  sw_hdrtmo_gain_ld_off2;
    unsigned char  sw_hdrtmo_gain_ld_off1;
    unsigned char  sw_hdrtmo_big_en;
    unsigned char  sw_hdrtmo_nobig_en;
    unsigned char  sw_hdrtmo_newhist_en;
    unsigned char  sw_hdrtmo_cnt_mode;
    unsigned short sw_hdrtmo_expl_lgratio;
    unsigned char  sw_hdrtmo_lgscl_ratio;
    unsigned char  sw_hdrtmo_cfg_alpha;
    unsigned short sw_hdrtmo_set_gainoff;
    unsigned short sw_hdrtmo_set_palpha;
    unsigned short sw_hdrtmo_set_lgmax;
    unsigned short sw_hdrtmo_set_lgmin;
    unsigned char  sw_hdrtmo_set_weightkey;
    unsigned short sw_hdrtmo_set_lgmean;
    unsigned short sw_hdrtmo_set_lgrange1;
    unsigned short sw_hdrtmo_set_lgrange0;
    unsigned short sw_hdrtmo_set_lgavgmax;
    unsigned char  sw_hdrtmo_clipgap1;
    unsigned char  sw_hdrtmo_clipgap0;
    unsigned char  sw_hdrtmo_clipratio1;
    unsigned char  sw_hdrtmo_clipratio0;
    unsigned char  sw_hdrtmo_ratiol;
    unsigned short sw_hdrtmo_lgscl_inv;
    unsigned short sw_hdrtmo_lgscl;
    unsigned short sw_hdrtmo_lgmax;
    unsigned short sw_hdrtmo_hist_low;
    unsigned short sw_hdrtmo_hist_min;
    unsigned char  sw_hdrtmo_hist_shift;
    unsigned short sw_hdrtmo_hist_0p3;
    unsigned short sw_hdrtmo_hist_high;
    unsigned short sw_hdrtmo_palpha_lwscl;
    unsigned short sw_hdrtmo_palpha_lw0p5;
    unsigned short sw_hdrtmo_palpha_0p18;
    unsigned short sw_hdrtmo_maxgain;
    unsigned short sw_hdrtmo_maxpalpha;
} TmoIOData_t;


typedef struct AhdrIOData_s
{
    MergeIOData_t MergeData;
    TmoIOData_t TmoData;
} AhdrIOData_t;

typedef AhdrIOData_t RkAiqAhdrProcResult_t;

typedef struct tmo_config_s
{
    float EnvLv[6];
    float GlobeLuma[6];
    float GlobeMaxLuma[6];

    float OEPdf[6];
    float DetailsHighLight[6];

    float enLumaWeight;
    float LumaWeight[225];
    float DarkPdfTH;
    float DarkTexturePdf[6];
    float DetailsLowLight[6];

    float enPosWeight;
    float PosWeight[225];//最多9块可以赋值为非0
    float PosCoef[6];
    float NoiseLowLight[6];

    float DynamicRange[6];
    float DayTh;
    float SmoothControlCoef1[6];
    float SmoothControlCoef2[6];

    float damp;
} tmo_config_t ;

typedef struct merge_config_s {
    int MergeMode;
    float EnvLv[6]; //0: dark 1:bright
    float MoveCoef[6];
    float OECurve_smooth[6];  //current over exposure curve slope
    float OECurve_offset[6];  //current over exposure curve offset
    float MDCurveLM_smooth[6];  //Move Detect curve slope betwwen long frame and middle frame
    float MDCurveLM_offset[6];  //Move Detect curve offset betwwen long frame and middle frame
    float MDCurveMS_smooth[6];  //Move Detect curve slope betwwen middle frame and short frame
    float MDCurveMS_offset[6];  //Move Detect curve slope betwwen middle frame and short frame
    float OECurve_damp;
    float MDCurveLM_damp;
    float MDCurveMS_damp ;
} merge_config_t;


typedef struct AhdrConfig_s {
    merge_config_t merge_para;
    tmo_config_t   tmo_para;
} AhdrConfig_t;


typedef struct tmoCtrlData_s
{
    float stCoef;
    float stCoefMax;
    float stCoefMin;
    int   stMax;
    int   stMin;
} tmoCtrlData_t;

typedef struct mgeCtrlData_S
{
    float stCoef;
    float stCoefMax;
    float stCoefMin;
    int   stSmthMax;
    int   stSmthMin;
    int   stOfstMax;
    int   stOfstMin;
} mgeCtrlData_t;

typedef struct amgeAttr_s
{
    mgeCtrlData_t stMDCurveLM;
    mgeCtrlData_t stMDCurveMS;
    mgeCtrlData_t stOECurve;
} amgeAttr_t;

typedef struct atmoAttr_S
{
    tmoCtrlData_t stGlobeMaxLuma;
    tmoCtrlData_t stGlobeLuma;
    tmoCtrlData_t stDtlsLL;
    tmoCtrlData_t stDtlsHL;
    tmoCtrlData_t stNosLL;
    bool  enSmthCtrl;
    tmoCtrlData_t stSmthCtrlCoef1;
    tmoCtrlData_t stSmthCtrlCoef2;
} atmoAttr_t;

typedef struct mmgeAttr_S
{
    float stMDCurveLM_smth;
    float stMDCurveLM_ofst;
    float stMDCurveMS_smth;
    float stMDCurveMS_ofst;
    float stOECurve_smth;
    float stOECurve_ofst;

} mmgeAttr_t;

typedef struct mtmoAttr_S
{
    float stGlobeMaxLuma;
    float stGlobeLuma;
    float stDtlsHL;
    float stDtlsLL;
    float stNosLL;
    bool  enSmthCtrl;
    int   stSmthCtrlCoef1;
    int   stSmthCtrlCoef2;

} mtmoAttr_t;

typedef struct ahdrAttr_S
{
    amgeAttr_t stMgeAuto;
    atmoAttr_t stTmoAuto;

} ahdrAttr_t;

typedef struct mhdrAttr_s
{
    mmgeAttr_t stMgeManual;
    mtmoAttr_t stTmoManual;
} mhdrAttr_t;

typedef enum OpMode_s {
    OpMode_Auto = 1,
    OpMode_MANU = 2
} OpMode_t;


typedef struct hdrAttr_s
{
    bool      bEnable;
    OpMode_t  opMode;
    ahdrAttr_t    stAuto;
    mhdrAttr_t stManual;
} hdrAttr_t;



#endif

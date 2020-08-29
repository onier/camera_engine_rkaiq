/*
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

#ifndef _RK_AIQ_CALIB_VERSION_H_
/*!
 * ==================== AIQ CALIB VERSION HISTORY ====================
 *
 * v1.0.0
 *  - initial version
 * v1.0.1
 * v1.1.0
 *  - add xml tag check
 * v1.1.1
 *  - magic code:996625
 *  - awb
 *  - difference tolerance and run interval in difference luma value
 * v1.1.2
 *  - magic code:998276
 *  - add COLOR_AS_GREY tag
 * v1.1.3
 *  - magic code:1003001
 *  - Aec: add sensor-AEC-LinearAECtrl-RawStatsEn, ae stats can use rawae/yuvae
 *  - Aec: add system-dcg_setting divide into 2 parts -normal /Hdr
 *  - Aec: add system-Gainrange-Linear/NonLinear, support db_mode
 * v1.1.4
 *  - magic code:1003993
 *  - Ahdr: add Band prior mode
 * v1.1.5
 *  - magic code: 1005025
 *  - sensor_para: add default sensor flip setting
 * v1.1.6
 *  - magic code:1011737
 *  - Aec: add AecEnvLvCalib
 *  - add ModuleInfo
 * v1.1.7
 *  - magic code:1023357
 *  - Adpcc: Redefine DPCC in IQ files, add Expert Mode
 *  - Adpcc: Add fast mode in DPCC
 * v1.1.8
 *  - magic code:1027323
 *  - Ahdr: remove Band_Prior from moresetting in IQ files
 *  - Ahdr: Add Band_Prior module in IQ files
 * v1.1.9
 *  - simplify params in Backlight, add MeasArea
 *  - add OverExpCtrl in AEC
 *  - delete useless iq params in AEC
 *  - including:AOE,Hist2Hal,AecRange,InternalAdjust
 * v1.1.a
 *  - ExpDelay support Hdr/Normal mode
 *  - Tolerance divide into ToleranceIn/ToleranceOut
 * v1.2.1
 *  - magic code:1007256
 *  - Ahdr:divide Ahdr into two parts,merge and tmo
 *  - Tmo: add linear tmo function
 * v1.2.2
 *  - Afec: add light_center and distortion_coeff
 * v1.2.3
 *  - LumaDetect: add the level2 of mutation threshold
 */


#define RK_AIQ_CALIB_VERSION_REAL_V          "v1.2.3"
#define RK_AIQ_CALIB_VERSION_MAGIC_V         "1011895"


/******* DO NOT EDIT THE FOLLOWINGS ***********/

#define RK_AIQ_CALIB_VERSION_HEAD            "Calib "
#define RK_AIQ_CALIB_VERSION_MAGIC_JOINT     ","
#define RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD "magicCode:"
#define RK_AIQ_CALIB_VERSION \
    RK_AIQ_CALIB_VERSION_HEAD\
    RK_AIQ_CALIB_VERSION_REAL_V\
    RK_AIQ_CALIB_VERSION_MAGIC_JOINT\
    RK_AIQ_CALIB_VERSION_MAGIC_CODE_HEAD\
    RK_AIQ_CALIB_VERSION_MAGIC_V

#endif

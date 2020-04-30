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

#ifndef _RK_AIQ_TYPES_H_
#define _RK_AIQ_TYPES_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "rk_aiq_comm.h"
#include "adebayer/rk_aiq_types_algo_adebayer.h"
#include "ae/rk_aiq_types_ae_algo.h"
#include "awb/rk_aiq_types_awb_algo.h"
#include "alsc/rk_aiq_types_alsc_algo.h"
#include "accm/rk_aiq_types_accm_algo.h"
#include "a3dlut/rk_aiq_types_a3dlut_algo.h"
#include "ahdr/rk_aiq_types_ahdr_algo_int.h"
#include "ahdr/rk_aiq_types_ahdr_algo.h"
#include "agamma/rk_aiq_types_agamma_algo.h"
#include "adehaze/rk_aiq_types_adehaze_algo.h"
#include "acp/rk_aiq_types_acp_algo.h"
#include "aie/rk_aiq_types_aie_algo_int.h"
#include "aorb/rk_aiq_types_orb_algo.h"
#include "asd/rk_aiq_types_asd_algo.h"
#include "anr/rk_aiq_types_anr_algo.h"
#include "anr/rk_aiq_types_anr_algo_int.h"
#include "asharp/rk_aiq_types_asharp_algo.h"
#include "asharp/rk_aiq_types_asharp_algo_int.h"
#include "adpcc/rk_aiq_types_adpcc_algo.h"
#include "adpcc/rk_aiq_types_adpcc_algo_int.h"
#include "ablc/rk_aiq_types_ablc_algo.h"
#include "ablc/rk_aiq_types_ablc_algo_int.h"
#include "agic/rk_aiq_types_algo_agic.h"
#include "aldch/rk_aiq_types_aldch_algo.h"
#include "afec/rk_aiq_types_afec_algo.h"

#ifdef RK_SIMULATOR_HW
#include "simulator/isp20_hw_simulator.h"
#include "af/rk_aiq_af_hw_v200.h"
#else
#include "af/rk_aiq_types_af_algo.h"
#include "adehaze/rk_aiq_types_adehaze_algo.h"

#endif

#define rk_fmt_fourcc(a, b, c, d)\
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
#define rk_fmt_fourcc_be(a, b, c, d)    (rk_fmt_fourcc(a, b, c, d) | (1 << 31))

typedef enum {
    /*      Pixel format         FOURCC                          depth  Description  */

    /* RGB formats */
    RK_PIX_FMT_RGB332 = rk_fmt_fourcc('R', 'G', 'B', '1'), /*  8  RGB-3-3-2     */
    RK_PIX_FMT_RGB444 = rk_fmt_fourcc('R', '4', '4', '4'), /* 16  xxxxrrrr ggggbbbb */
    RK_PIX_FMT_ARGB444 = rk_fmt_fourcc('A', 'R', '1', '2'), /* 16  aaaarrrr ggggbbbb */
    RK_PIX_FMT_XRGB444 = rk_fmt_fourcc('X', 'R', '1', '2'), /* 16  xxxxrrrr ggggbbbb */
    RK_PIX_FMT_RGB555 = rk_fmt_fourcc('R', 'G', 'B', 'O'), /* 16  RGB-5-5-5     */
    RK_PIX_FMT_ARGB555 = rk_fmt_fourcc('A', 'R', '1', '5'), /* 16  ARGB-1-5-5-5  */
    RK_PIX_FMT_XRGB555 = rk_fmt_fourcc('X', 'R', '1', '5'), /* 16  XRGB-1-5-5-5  */
    RK_PIX_FMT_RGB565 = rk_fmt_fourcc('R', 'G', 'B', 'P'), /* 16  RGB-5-6-5     */
    RK_PIX_FMT_RGB555X = rk_fmt_fourcc('R', 'G', 'B', 'Q'), /* 16  RGB-5-5-5 BE  */
    RK_PIX_FMT_ARGB555X = rk_fmt_fourcc_be('A', 'R', '1', '5'), /* 16  ARGB-5-5-5 BE */
    RK_PIX_FMT_XRGB555X = rk_fmt_fourcc_be('X', 'R', '1', '5'), /* 16  XRGB-5-5-5 BE */
    RK_PIX_FMT_RGB565X = rk_fmt_fourcc('R', 'G', 'B', 'R'), /* 16  RGB-5-6-5 BE  */
    RK_PIX_FMT_BGR666 = rk_fmt_fourcc('B', 'G', 'R', 'H'), /* 18  BGR-6-6-6   */
    RK_PIX_FMT_BGR24  = rk_fmt_fourcc('B', 'G', 'R', '3'), /* 24  BGR-8-8-8     */
    RK_PIX_FMT_RGB24   = rk_fmt_fourcc('R', 'G', 'B', '3'), /* 24  RGB-8-8-8     */
    RK_PIX_FMT_BGR32  = rk_fmt_fourcc('B', 'G', 'R', '4'), /* 32  BGR-8-8-8-8   */
    RK_PIX_FMT_ABGR32  = rk_fmt_fourcc('A', 'R', '2', '4'), /* 32  BGRA-8-8-8-8  */
    RK_PIX_FMT_XBGR32 = rk_fmt_fourcc('X', 'R', '2', '4'), /* 32  BGRX-8-8-8-8  */
    RK_PIX_FMT_RGB32 = rk_fmt_fourcc('R', 'G', 'B', '4'), /* 32  RGB-8-8-8-8   */
    RK_PIX_FMT_ARGB32 = rk_fmt_fourcc('B', 'A', '2', '4'), /* 32  ARGB-8-8-8-8  */
    RK_PIX_FMT_XRGB32 = rk_fmt_fourcc('B', 'X', '2', '4'), /* 32  XRGB-8-8-8-8  */

    /* Grey formats */
    RK_PIX_FMT_GREY  = rk_fmt_fourcc('G', 'R', 'E', 'Y'), /*  8  Greyscale     */
    RK_PIX_FMT_Y4   = rk_fmt_fourcc('Y', '0', '4', ' '), /*  4  Greyscale     */
    RK_PIX_FMT_Y6   = rk_fmt_fourcc('Y', '0', '6', ' '), /*  6  Greyscale     */
    RK_PIX_FMT_Y10  = rk_fmt_fourcc('Y', '1', '0', ' '), /* 10  Greyscale     */
    RK_PIX_FMT_Y12  = rk_fmt_fourcc('Y', '1', '2', ' '), /* 12  Greyscale     */
    RK_PIX_FMT_Y16  = rk_fmt_fourcc('Y', '1', '6', ' '), /* 16  Greyscale     */
    RK_PIX_FMT_Y16_BE = rk_fmt_fourcc_be('Y', '1', '6', ' '), /* 16  Greyscale BE  */

    /* Grey bit-packed formats */
    RK_PIX_FMT_Y10BPACK  = rk_fmt_fourcc('Y', '1', '0', 'B'), /* 10  Greyscale bit-packed */

    /* Palette formats */
    RK_PIX_FMT_PAL8  = rk_fmt_fourcc('P', 'A', 'L', '8'), /*  8  8-bit palette */

    /* Chrominance formats */
    RK_PIX_FMT_UV8  = rk_fmt_fourcc('U', 'V', '8', ' '), /*  8  UV 4:4 */

    /* Luminance+Chrominance formats */
    RK_PIX_FMT_YVU410 = rk_fmt_fourcc('Y', 'V', 'U', '9'), /*  9  YVU 4:1:0     */
    RK_PIX_FMT_YVU420 = rk_fmt_fourcc('Y', 'V', '1', '2'), /* 12  YVU 4:2:0     */
    RK_PIX_FMT_YUYV  = rk_fmt_fourcc('Y', 'U', 'Y', 'V'), /* 16  YUV 4:2:2     */
    RK_PIX_FMT_YYUV  = rk_fmt_fourcc('Y', 'Y', 'U', 'V'), /* 16  YUV 4:2:2     */
    RK_PIX_FMT_YVYU  = rk_fmt_fourcc('Y', 'V', 'Y', 'U'), /* 16 YVU 4:2:2 */
    RK_PIX_FMT_UYVY  = rk_fmt_fourcc('U', 'Y', 'V', 'Y'), /* 16  YUV 4:2:2     */
    RK_PIX_FMT_VYUY  = rk_fmt_fourcc('V', 'Y', 'U', 'Y'), /* 16  YUV 4:2:2     */
    RK_PIX_FMT_YUV422P = rk_fmt_fourcc('4', '2', '2', 'P'), /* 16  YVU422 planar */
    RK_PIX_FMT_YUV411P = rk_fmt_fourcc('4', '1', '1', 'P'), /* 16  YVU411 planar */
    RK_PIX_FMT_Y41P  = rk_fmt_fourcc('Y', '4', '1', 'P'), /* 12  YUV 4:1:1     */
    RK_PIX_FMT_YUV444 = rk_fmt_fourcc('Y', '4', '4', '4'), /* 16  xxxxyyyy uuuuvvvv */
    RK_PIX_FMT_YUV555 = rk_fmt_fourcc('Y', 'U', 'V', 'O'), /* 16  YUV-5-5-5     */
    RK_PIX_FMT_YUV565 = rk_fmt_fourcc('Y', 'U', 'V', 'P'), /* 16  YUV-5-6-5     */
    RK_PIX_FMT_YUV32  = rk_fmt_fourcc('Y', 'U', 'V', '4'), /* 32  YUV-8-8-8-8   */
    RK_PIX_FMT_YUV410 = rk_fmt_fourcc('Y', 'U', 'V', '9'), /*  9  YUV 4:1:0     */
    RK_PIX_FMT_YUV420 = rk_fmt_fourcc('Y', 'U', '1', '2'), /* 12  YUV 4:2:0     */
    RK_PIX_FMT_HI240  = rk_fmt_fourcc('H', 'I', '2', '4'), /*  8  8-bit color   */
    RK_PIX_FMT_HM12  = rk_fmt_fourcc('H', 'M', '1', '2'), /*  8  YUV 4:2:0 16x16 macroblocks */
    RK_PIX_FMT_M420  = rk_fmt_fourcc('M', '4', '2', '0'), /* 12  YUV 4:2:0 2 lines y, 1 line uv interleaved */

    /* two planes -- one Y, one Cr + Cb interleaved  */
    RK_PIX_FMT_NV12  = rk_fmt_fourcc('N', 'V', '1', '2'), /* 12  Y/CbCr 4:2:0  */
    RK_PIX_FMT_NV21  = rk_fmt_fourcc('N', 'V', '2', '1'), /* 12  Y/CrCb 4:2:0  */
    RK_PIX_FMT_NV16  = rk_fmt_fourcc('N', 'V', '1', '6'), /* 16  Y/CbCr 4:2:2  */
    RK_PIX_FMT_NV61  = rk_fmt_fourcc('N', 'V', '6', '1'), /* 16  Y/CrCb 4:2:2  */
    RK_PIX_FMT_NV24  = rk_fmt_fourcc('N', 'V', '2', '4'), /* 24  Y/CbCr 4:4:4  */
    RK_PIX_FMT_NV42  = rk_fmt_fourcc('N', 'V', '4', '2'), /* 24  Y/CrCb 4:4:4  */

    /* two non contiguous planes - one Y, one Cr + Cb interleaved  */
    RK_PIX_FMT_NV12M = rk_fmt_fourcc('N', 'M', '1', '2'), /* 12  Y/CbCr 4:2:0  */
    RK_PIX_FMT_NV21M = rk_fmt_fourcc('N', 'M', '2', '1'), /* 21  Y/CrCb 4:2:0  */
    RK_PIX_FMT_NV16M = rk_fmt_fourcc('N', 'M', '1', '6'), /* 16  Y/CbCr 4:2:2  */
    RK_PIX_FMT_NV61M = rk_fmt_fourcc('N', 'M', '6', '1'), /* 16  Y/CrCb 4:2:2  */
    RK_PIX_FMT_NV12MT = rk_fmt_fourcc('T', 'M', '1', '2'), /* 12  Y/CbCr 4:2:0 64x32 macroblocks */
    RK_PIX_FMT_NV12MT_16X16 = rk_fmt_fourcc('V', 'M', '1', '2'), /* 12  Y/CbCr 4:2:0 16x16 macroblocks */

    /* three non contiguous planes - Y, Cb, Cr */
    RK_PIX_FMT_YUV420M = rk_fmt_fourcc('Y', 'M', '1', '2'), /* 12  YUV420 planar */
    RK_PIX_FMT_YVU420M = rk_fmt_fourcc('Y', 'M', '2', '1'), /* 12  YVU420 planar */
    RK_PIX_FMT_YUV422M = rk_fmt_fourcc('Y', 'M', '1', '6'), /* 16  YUV422 planar */
    RK_PIX_FMT_YVU422M = rk_fmt_fourcc('Y', 'M', '6', '1'), /* 16  YVU422 planar */
    RK_PIX_FMT_YUV444M = rk_fmt_fourcc('Y', 'M', '2', '4'), /* 24  YUV444 planar */
    RK_PIX_FMT_YVU444M = rk_fmt_fourcc('Y', 'M', '4', '2'), /* 24  YVU444 planar */

    /* Bayer formats - see http://www.siliconimaging.com/RGB%20Bayer.htm */
    RK_PIX_FMT_SBGGR8 = rk_fmt_fourcc('B', 'A', '8', '1'), /*  8  BGBG.. GRGR.. */
    RK_PIX_FMT_SGBRG8 = rk_fmt_fourcc('G', 'B', 'R', 'G'), /*  8  GBGB.. RGRG.. */
    RK_PIX_FMT_SGRBG8 = rk_fmt_fourcc('G', 'R', 'B', 'G'), /*  8  GRGR.. BGBG.. */
    RK_PIX_FMT_SRGGB8 = rk_fmt_fourcc('R', 'G', 'G', 'B'), /*  8  RGRG.. GBGB.. */
    RK_PIX_FMT_SBGGR10 = rk_fmt_fourcc('B', 'G', '1', '0'), /* 10  BGBG.. GRGR.. */
    RK_PIX_FMT_SGBRG10 = rk_fmt_fourcc('G', 'B', '1', '0'), /* 10  GBGB.. RGRG.. */
    RK_PIX_FMT_SGRBG10 = rk_fmt_fourcc('B', 'A', '1', '0'), /* 10  GRGR.. BGBG.. */
    RK_PIX_FMT_SRGGB10 = rk_fmt_fourcc('R', 'G', '1', '0'), /* 10  RGRG.. GBGB.. */
    /* 10bit raw bayer packed, 5 bytes for every 4 pixels */
    RK_PIX_FMT_SBGGR10P = rk_fmt_fourcc('p', 'B', 'A', 'A'),
    RK_PIX_FMT_SGBRG10P = rk_fmt_fourcc('p', 'G', 'A', 'A'),
    RK_PIX_FMT_SGRBG10P = rk_fmt_fourcc('p', 'g', 'A', 'A'),
    RK_PIX_FMT_SRGGB10P = rk_fmt_fourcc('p', 'R', 'A', 'A'),
    /* 10bit raw bayer a-law compressed to 8 bits */
    RK_PIX_FMT_SBGGR10ALAW8 = rk_fmt_fourcc('a', 'B', 'A', '8'),
    RK_PIX_FMT_SGBRG10ALAW8 = rk_fmt_fourcc('a', 'G', 'A', '8'),
    RK_PIX_FMT_SGRBG10ALAW8 = rk_fmt_fourcc('a', 'g', 'A', '8'),
    RK_PIX_FMT_SRGGB10ALAW8 = rk_fmt_fourcc('a', 'R', 'A', '8'),
    /* 10bit raw bayer DPCM compressed to 8 bits */
    RK_PIX_FMT_SBGGR10DPCM8 = rk_fmt_fourcc('b', 'B', 'A', '8'),
    RK_PIX_FMT_SGBRG10DPCM8 = rk_fmt_fourcc('b', 'G', 'A', '8'),
    RK_PIX_FMT_SGRBG10DPCM8 = rk_fmt_fourcc('B', 'D', '1', '0'),
    RK_PIX_FMT_SRGGB10DPCM8 = rk_fmt_fourcc('b', 'R', 'A', '8'),
    RK_PIX_FMT_SBGGR12 = rk_fmt_fourcc('B', 'G', '1', '2'), /* 12  BGBG.. GRGR.. */
    RK_PIX_FMT_SGBRG12 = rk_fmt_fourcc('G', 'B', '1', '2'), /* 12  GBGB.. RGRG.. */
    RK_PIX_FMT_SGRBG12 = rk_fmt_fourcc('B', 'A', '1', '2'), /* 12  GRGR.. BGBG.. */
    RK_PIX_FMT_SRGGB12 = rk_fmt_fourcc('R', 'G', '1', '2'), /* 12  RGRG.. GBGB.. */
    RK_PIX_FMT_SBGGR16 = rk_fmt_fourcc('B', 'Y', 'R', '2'), /* 16  BGBG.. GRGR.. */

    /* compressed formats */
    RK_PIX_FMT_MJPEG = rk_fmt_fourcc('M', 'J', 'P', 'G'), /* Motion-JPEG   */
    RK_PIX_FMT_JPEG  = rk_fmt_fourcc('J', 'P', 'E', 'G'), /* JFIF JPEG     */
    RK_PIX_FMT_DV   = rk_fmt_fourcc('d', 'v', 's', 'd'), /* 1394          */
    RK_PIX_FMT_MPEG  = rk_fmt_fourcc('M', 'P', 'E', 'G'), /* MPEG-1/2/4 Multiplexed */
    RK_PIX_FMT_H264  = rk_fmt_fourcc('H', '2', '6', '4'), /* H264 with start codes */
    RK_PIX_FMT_H264_NO_SC = rk_fmt_fourcc('A', 'V', 'C', '1'), /* H264 without start codes */
    RK_PIX_FMT_H264_MVC = rk_fmt_fourcc('M', '2', '6', '4'), /* H264 MVC */
    RK_PIX_FMT_H264_SLICE = rk_fmt_fourcc('S', '2', '6', '4'), /* H264 parsed slices */
    RK_PIX_FMT_H263   = rk_fmt_fourcc('H', '2', '6', '3'), /* H263          */
    RK_PIX_FMT_MPEG1 = rk_fmt_fourcc('M', 'P', 'G', '1'), /* MPEG-1 ES     */
    RK_PIX_FMT_MPEG2   = rk_fmt_fourcc('M', 'P', 'G', '2'), /* MPEG-2 ES     */
    RK_PIX_FMT_MPEG4   = rk_fmt_fourcc('M', 'P', 'G', '4'), /* MPEG-4 part 2 ES */
    RK_PIX_FMT_XVID    = rk_fmt_fourcc('X', 'V', 'I', 'D'), /* Xvid           */
    RK_PIX_FMT_VC1_ANNEX_G = rk_fmt_fourcc('V', 'C', '1', 'G'), /* SMPTE 421M Annex G compliant stream */
    RK_PIX_FMT_VC1_ANNEX_L = rk_fmt_fourcc('V', 'C', '1', 'L'), /* SMPTE 421M Annex L compliant stream */
    RK_PIX_FMT_VP8     = rk_fmt_fourcc('V', 'P', '8', '0'), /* VP8 */
    RK_PIX_FMT_VP8_FRAME = rk_fmt_fourcc('V', 'P', '8', 'F'), /* VP8 parsed frames */

    /*  Vendor-specific formats   */
    RK_PIX_FMT_CPIA1   = rk_fmt_fourcc('C', 'P', 'I', 'A'), /* cpia1 YUV */
    RK_PIX_FMT_WNVA    = rk_fmt_fourcc('W', 'N', 'V', 'A'), /* Winnov hw compress */
    RK_PIX_FMT_SN9C10X = rk_fmt_fourcc('S', '9', '1', '0'), /* SN9C10x compression */
    RK_PIX_FMT_SN9C20X_I420 = rk_fmt_fourcc('S', '9', '2', '0'), /* SN9C20x YUV 4:2:0 */
    RK_PIX_FMT_PWC1    = rk_fmt_fourcc('P', 'W', 'C', '1'), /* pwc older webcam */
    RK_PIX_FMT_PWC2    = rk_fmt_fourcc('P', 'W', 'C', '2'), /* pwc newer webcam */
    RK_PIX_FMT_ET61X251 = rk_fmt_fourcc('E', '6', '2', '5'), /* ET61X251 compression */
    RK_PIX_FMT_SPCA501 = rk_fmt_fourcc('S', '5', '0', '1'), /* YUYV per line */
    RK_PIX_FMT_SPCA505 = rk_fmt_fourcc('S', '5', '0', '5'), /* YYUV per line */
    RK_PIX_FMT_SPCA508 = rk_fmt_fourcc('S', '5', '0', '8'), /* YUVY per line */
    RK_PIX_FMT_SPCA561 = rk_fmt_fourcc('S', '5', '6', '1'), /* compressed GBRG bayer */
    RK_PIX_FMT_PAC207  = rk_fmt_fourcc('P', '2', '0', '7'), /* compressed BGGR bayer */
    RK_PIX_FMT_MR97310A = rk_fmt_fourcc('M', '3', '1', '0'), /* compressed BGGR bayer */
    RK_PIX_FMT_JL2005BCD = rk_fmt_fourcc('J', 'L', '2', '0'), /* compressed RGGB bayer */
    RK_PIX_FMT_SN9C2028 = rk_fmt_fourcc('S', 'O', 'N', 'X'), /* compressed GBRG bayer */
    RK_PIX_FMT_SQ905C  = rk_fmt_fourcc('9', '0', '5', 'C'), /* compressed RGGB bayer */
    RK_PIX_FMT_PJPG    = rk_fmt_fourcc('P', 'J', 'P', 'G'), /* Pixart 73xx JPEG */
    RK_PIX_FMT_OV511   = rk_fmt_fourcc('O', '5', '1', '1'), /* ov511 JPEG */
    RK_PIX_FMT_OV518   = rk_fmt_fourcc('O', '5', '1', '8'), /* ov518 JPEG */
    RK_PIX_FMT_STV0680 = rk_fmt_fourcc('S', '6', '8', '0'), /* stv0680 bayer */
    RK_PIX_FMT_TM6000  = rk_fmt_fourcc('T', 'M', '6', '0'), /* tm5600/tm60x0 */
    RK_PIX_FMT_CIT_YYVYUY = rk_fmt_fourcc('C', 'I', 'T', 'V'), /* one line of Y then 1 line of VYUY */
    RK_PIX_FMT_KONICA420 = rk_fmt_fourcc('K', 'O', 'N', 'I'), /* YUV420 planar in blocks of 256 pixels */
    RK_PIX_FMT_JPGL      =  rk_fmt_fourcc('J', 'P', 'G', 'L'), /* JPEG-Lite */
    RK_PIX_FMT_SE401     = rk_fmt_fourcc('S', '4', '0', '1'), /* se401 janggu compressed rgb */
    RK_PIX_FMT_S5C_UYVY_JPG = rk_fmt_fourcc('S', '5', 'C', 'I'), /* S5C73M3 interleaved UYVY/JPEG */
    RK_PIX_FMT_Y8I     = rk_fmt_fourcc('Y', '8', 'I', ' '), /* Greyscale 8-bit L/R interleaved */
    RK_PIX_FMT_Y12I    = rk_fmt_fourcc('Y', '1', '2', 'I'), /* Greyscale 12-bit L/R interleaved */
    RK_PIX_FMT_Z16     = rk_fmt_fourcc('Z', '1', '6', ' '), /* Depth data 16-bit */
} rk_aiq_format_t;

typedef struct rk_frame_fmt_s {
    int32_t width;
    int32_t height;
    rk_aiq_format_t format;
} rk_frame_fmt_t;

typedef struct {
    char sensor_name[32];
    rk_frame_fmt_t  *support_fmt;
    int num;
} rk_aiq_sensor_info_t;

typedef struct {
    int UNKNOWN;
} rk_aiq_lens_info_t;

typedef struct {
    rk_aiq_sensor_info_t    sensor_info;
    rk_aiq_lens_info_t      lens_info;
    // supported Antibanding modes
    // supported lock modes
    // supported ae compensation range/step
    // supported ae measure mode
    // supported af modes
    // other from iq
} rk_aiq_static_info_t;

// sensor
typedef struct {
    unsigned short line_periods_vertical_blanking;
    unsigned short fine_integration_time_min;
    unsigned short fine_integration_time_max_margin;
    unsigned short coarse_integration_time_min;
    unsigned short coarse_integration_time_max_margin;
    unsigned int frame_length_lines;
    unsigned int line_length_pck;
    unsigned int vt_pix_clk_freq_hz;
    float pixel_clock_freq_mhz;
    unsigned short pixel_periods_per_line;
    unsigned short line_periods_per_field;
    uint32_t sensor_output_width;
    uint32_t sensor_output_height;
    uint32_t sensor_pixelformat;
    uint32_t isp_acq_width;
    uint32_t isp_acq_height;
} rk_aiq_exposure_sensor_descriptor;

// exposure
typedef RkAiqExpParam_t rk_aiq_exposure_real_params;
typedef RkAiqExpSensorParam_t rk_aiq_exposure_sensor_params;
typedef RkAiqExpParamComb_t rk_aiq_exposure_params_comb_t;
typedef RKAiqAecExpInfo_t rk_aiq_exposure_params_t;

// focus
typedef struct
{
    unsigned int next_lens_pos;
} rk_aiq_focus_params_t;

// isp

struct rk_aiq_isp_window {
    uint16_t h_offs;
    uint16_t v_offs;
    uint16_t h_size;
    uint16_t v_size;
} __attribute__ ((packed));


typedef RKAiqAecStats_t rk_aiq_isp_aec_stats_t;


#ifdef RK_SIMULATOR_HW
typedef rawaf_isp_af_stat_t rk_aiq_isp_af_stats_t;
typedef rawaf_isp_af_meas_t rk_aiq_isp_af_meas_t;
typedef rawaf_focus_pos_meas_t rk_aiq_af_focus_pos_meas_t;
#else
typedef rk_aiq_af_algo_stat_t rk_aiq_isp_af_stats_t;
typedef rk_aiq_af_algo_meas_t rk_aiq_isp_af_meas_t;
typedef rk_aiq_af_algo_focus_pos_t rk_aiq_af_focus_pos_meas_t;
#endif

typedef rk_aiq_ae_meas_params_t rk_aiq_isp_aec_meas_t;
typedef rk_aiq_hist_meas_params_t rk_aiq_isp_hist_meas_t;

/**wb gain **/

#ifdef RK_SIMULATOR_HW
typedef sim_orb_stat_t rk_aiq_isp_orb_stats_t;
#else
typedef rk_aiq_orb_algo_stat_t rk_aiq_isp_orb_stats_t;
#endif
typedef rk_aiq_orb_algo_meas_t rk_aiq_isp_orb_meas_t;
typedef struct {
    int orb_en;
    int limit_value;
    int max_feature;
} rk_aiq_isp_orb_t;

#if 0
typedef struct {

} rk_aiq_isp_blc_t;
#endif

#if 0
typedef struct {

} rk_aiq_isp_dpcc_t;
#endif

typedef struct {
    int UNKNOWN;
} rk_aiq_isp_drc_t;

typedef AgicConfig_t rk_aiq_isp_gic_t;

/*typedef struct {

} rk_aiq_isp_lsc_t;*/

/*
typedef struct {
    unsigned char filter_c_en;
    unsigned char filter_g_en;

    unsigned char gain_offset;
    unsigned short hf_offset;
    unsigned char thed1;
    unsigned char thed0;
    unsigned char dist_scale;
    unsigned char max_ratio;
    unsigned char clip_en;

    signed char filter1_coe[5];
    signed char filter2_coe[5];

    unsigned char offset;
    unsigned char shift_num;
    unsigned char order_max;
    unsigned char order_min;
} rk_aiq_isp_demosaic_t;
*/
typedef AdebayerConfig_t rk_aiq_isp_demosaic_t;

/*
typedef struct {

    int enhance_en;
    int hist_chn;
    int hpara_en;
    int hist_en;
    int dc_en;
    int big_en;
    int nobig_en;

    unsigned char yblk_th;
    unsigned charyhist_th;
    unsigned char dc_max_th;
    unsigned char dc_min_th;

    unsigned char wt_max;
    unsigned char bright_max;
    unsigned char bright_min;

    unsigned char tmax_base;
    unsigned char chardark_th;
    unsigned char air_max;
    unsigned char air_min;

    unsigned short tmax_max;
    unsigned short tmax_off;

    unsigned char hist_th_off;
    unsigned char hist_gratio;

    unsigned short hist_min;
    unsigned short hist_k;

    unsigned short enhance_value;
    unsigned short hist_scale;

    unsigned short iir_wt_sigma;
    unsigned short iir_sigma;
    unsigned short stab_fnum;

    unsigned short iir_tmax_sigma;
    unsigned short iir_air_sigma;

    unsigned short cfg_wt;
    unsigned short cfg_air;
    unsigned short cfg_alpha;

    unsigned short cfg_gratio;
    unsigned short cfg_tmax;

    unsigned short dc_weitcur;
    unsigned short dc_thed;

    unsigned short sw_dhaz_dc_bf_h3;
    unsigned short sw_dhaz_dc_bf_h2;
    unsigned short sw_dhaz_dc_bf_h1;
    unsigned short sw_dhaz_dc_bf_h0;

    unsigned short sw_dhaz_dc_bf_h5;
    unsigned short sw_dhaz_dc_bf_h4;

    int air_weitcur;
    int air_thed;

    unsigned short air_bf_h2;
    unsigned short air_bf_h1;
    unsigned short air_bf_h0;

    unsigned short gaus_h2;
    unsigned short gaus_h1;
    unsigned short gaus_h0;

    unsigned short conv_t0[6];
    unsigned short conv_t1[6];
    unsigned short conv_t2[6];

    int dhaz_adp_air_base;
    int dhaz_adp_wt;

    int dhaz_adp_gratio;
    int dhaz_adp_tmax;

    int h_r_iir[64];
    int h_g_iir[64];
    int h_b_iir[64];


} rk_aiq_isp_dehaze_t;*/





//typedef rk_aiq_gamma_cfg_t rk_aiq_gamma_cfg_t;


typedef struct {
    unsigned char  equ_segm;
    unsigned short offset;
    unsigned short gamma_y[45];
} rk_aiq_isp_goc_t;

typedef struct {
    int UNKNOWN;
} rk_aiq_isp_wdr_t;

typedef struct {
    int UNKNOWN;
} rk_aiq_isp_csm_t;

typedef struct {
    int UNKNOWN;
} rk_aiq_isp_conv422_t;

typedef struct {
    int UNKNOWN;
} rk_aiq_isp_yuvconv_t;

typedef struct {
    int ldch_en;
    int lut_h_size;
    int lut_v_size;
    int lut_size;
    unsigned short lut_mapxy[LDCH_MESH_XY_NUM_ALGO];
} rk_aiq_isp_ldch_t;

typedef struct {
    unsigned char fec_en;
    unsigned char crop_en;
    unsigned int crop_width;
    unsigned int crop_height;
    unsigned char mesh_density;
    unsigned int mesh_size;
    unsigned short sw_mesh_xi[FEC_MESH_XY_NUM_ALGO];
    unsigned char sw_mesh_xf[FEC_MESH_XY_NUM_ALGO];
    unsigned short sw_mesh_yi[FEC_MESH_XY_NUM_ALGO];
    unsigned char sw_mesh_yf[FEC_MESH_XY_NUM_ALGO];
} rk_aiq_isp_fec_t;

#if 0
typedef struct {

} rk_aiq_isp_tnr_t;

typedef struct {

} rk_aiq_isp_ynr_t;


typedef struct {

} rk_aiq_isp_uvnr_t;

typedef struct {

} rk_aiq_isp_sharpen_t;

typedef struct {

} rk_aiq_isp_edgeflt_t;
#endif

typedef struct {
    int UNKNOWN;
} rk_aiq_isp_cgc_t;

#if 0
typedef struct {

} rk_aiq_isp_rawnr_t;
#endif


typedef struct {
    rk_aiq_isp_aec_stats_t* aec_stats;
    rk_aiq_awb_stat_res_v200_t* awb_stats_v200;
    rk_aiq_isp_af_stats_t*  af_stats;
} rk_aiq_isp_stats_t;

typedef RkAiqAhdrProcResult_t rk_aiq_isp_hdr_t;

#if 1
typedef RKAnr_Bayernr_Fix_t rk_aiq_isp_rawnr_t;
typedef RKAnr_Mfnr_Fix_t rk_aiq_isp_tnr_t;
typedef RKAnr_Ynr_Fix_t rk_aiq_isp_ynr_t;
typedef RKAnr_Uvnr_Fix_t rk_aiq_isp_uvnr_t;
typedef ANRProcResult_t rkaiq_anr_procRes_t;
#endif


#if 1
typedef RKAsharp_Sharp_Fix_t rk_aiq_isp_sharpen_t;
typedef RKAsharp_Edgefilter_Fix_t rk_aiq_isp_edgeflt_t;
typedef AsharpProcResult_t rkaiq_asharp_procRes_t;
#endif

typedef AblcProcResult_t rk_aiq_isp_blc_t;
typedef AdpccProcResult_t rk_aiq_isp_dpcc_t;

typedef RKAnr_Gain_Fix_t rk_aiq_isp_gain_t;

#if 0
typedef struct {
    uint32_t module_enable_mask;
    rk_aiq_isp_aec_meas_t*   aec_meas;
    rk_aiq_isp_awb_meas_t*   awb_meas;
    rk_aiq_isp_af_meas_t*    af_meas;
    rk_aiq_isp_blc_t*        blc;
    rk_aiq_isp_dpcc_t*       dpcc;
    rk_aiq_isp_hdr_t*        hdr;
    rk_aiq_isp_rawnr_t*      rawnr;
    rk_aiq_isp_drc_t*        drc;
    rk_aiq_isp_gic_t*        gic;
    rk_aiq_isp_lsc_t*        lsc;
    rk_aiq_isp_demosaic_t*   demosaic;
    rk_aiq_isp_ldch_t*       ldch;
    rk_aiq_isp_lut3d_t*      lut3d;
    rk_aiq_isp_dehaze_t*     dehaze;
    rk_aiq_isp_ccm_t*        ccm;
    rk_aiq_isp_goc_t*        goc;
    rk_aiq_isp_wdr_t*        wdr;
    rk_aiq_isp_csm_t*        csm;
    rk_aiq_isp_tnr_t*        tnr;
    rk_aiq_isp_ynr_t*        ynr;
    rk_aiq_isp_uvnr_t*       uvnr;
    rk_aiq_isp_sharpen_t*    sharpen;
    rk_aiq_isp_edgeflt_t*    edgeflt;
    rk_aiq_isp_cgc_t*        cgc;
    rk_aiq_isp_conv422_t*    conv22;
    rk_aiq_isp_yuvconv_t*    yuvconv;
} rk_aiq_isp_partial_params_t;
#endif

typedef struct rk_aiq_isp_ie_s {
    rk_aiq_aie_params_t base;
    rk_aiq_aie_params_int_t extra;
} rk_aiq_isp_ie_t;

typedef struct {
    uint32_t module_enable_mask;
    sint32_t frame_id;
    rk_aiq_isp_aec_meas_t   aec_meas;
    rk_aiq_awb_stat_cfg_v200_t   awb_cfg_v200;
    rk_aiq_awb_stat_cfg_v201_t   awb_cfg_v201;
    rk_aiq_wb_gain_t       awb_gain;
    rk_aiq_isp_af_meas_t    af_meas;
    rk_aiq_isp_hist_meas_t  hist_meas;
    rk_aiq_isp_blc_t        blc;
    rk_aiq_isp_dpcc_t       dpcc;
    RkAiqAhdrProcResult_t   ahdr_proc_res;//porc data for hw/simulator
    rk_aiq_isp_rawnr_t      rawnr;
    rk_aiq_isp_drc_t        drc;
    rk_aiq_isp_gic_t        gic;
    rk_aiq_lsc_cfg_t        lsc;
    rk_aiq_isp_demosaic_t   demosaic;
    rk_aiq_isp_ldch_t       ldch;
    //rk_aiq_isp_fec_t        fec;
    rk_aiq_lut3d_cfg_t      lut3d;
    //rk_aiq_isp_dehaze_t     dehaze;
    rk_aiq_dehaze_cfg_t     adhaz_config;
    rk_aiq_ccm_cfg_t        ccm;
    //rk_aiq_isp_goc_t        goc;
    rk_aiq_gamma_cfg_t      agamma_config;
    rk_aiq_isp_wdr_t        wdr;
    rk_aiq_isp_csm_t        csm;
    rk_aiq_isp_cgc_t        cgc;
    rk_aiq_isp_conv422_t    conv22;
    rk_aiq_isp_yuvconv_t    yuvconv;
	rk_aiq_isp_gain_t       gain_config;
    //anr result
    rkaiq_anr_procRes_t     rkaiq_anr_proc_res;
    rkaiq_asharp_procRes_t  rkaiq_asharp_proc_res;
    rk_aiq_acp_params_t     cp;
    rk_aiq_isp_ie_t         ie;
#ifdef RK_SIMULATOR_HW
    //rk_aiq_awb_stat_cfg_v200_t  awb_hw0_para;
    // rk_aiq_awb_stat_cfg_v201_t  awb_hw1_para;
    //rk_aiq_wb_gain_t        awb_gain_algo;
    // float                   awb_smooth_factor;
    //adhaz

    //agamma
    //rk_aiq_gamma_cfg_t  agamma_config;



    //ahdr
    //RkAiqAhdrProcResult_t   ahdr_proc_res;

    //adpcc
    //rkaiq_adpcc_procRes_t rkaiq_adpcc_proc_res;

    //adebayer
    //AdebayerConfig_t adebayer_config;

    //ablc
    //rk_aiq_isp_blc_t rkaiq_ablc_proc_res;
#endif
} rk_aiq_isp_params_t;

#define RKAIQ_ISPP_TNR_ID			(1 << 0)
#define RKAIQ_ISPP_NR_ID			(1 << 1)
#define RKAIQ_ISPP_SHARP_ID			(1 << 2)
#define RKAIQ_ISPP_FEC_ID			(1 << 3)
#define RKAIQ_ISPP_ORB_ID			(1 << 4)

typedef struct {
    uint32_t update_mask;
    sint32_t frame_id;
    rk_aiq_isp_tnr_t        tnr;
    rk_aiq_isp_ynr_t        ynr;
    rk_aiq_isp_uvnr_t       uvnr;
    rk_aiq_isp_sharpen_t    sharpen;
    rk_aiq_isp_edgeflt_t    edgeflt;
    rk_aiq_isp_orb_t        orb;
    rk_aiq_isp_fec_t        fec;
} rk_aiq_ispp_params_t;

#endif

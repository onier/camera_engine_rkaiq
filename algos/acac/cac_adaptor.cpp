/*
 * cac_algo_adaptor.h
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd.
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#include "cac_adaptor.h"

#include <fstream>
#include <iostream>

#include "rk_aiq_types_priv.h"
#include "xcam_log.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace XCam;

namespace RkCam {

#define BITS_PER_BYTE  8
#define BYTES_PER_WORD 4
#define BITS_PER_WORD  (BITS_PER_BYTE * BYTES_PER_WORD)

#define INTERP_CAC(x0, x1, ratio) static_cast<int>((ratio) * ((x1) - (x0)) + (x0))

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

enum class LutBufferState {
    kInitial   = MESH_BUF_INIT,
    kWait2Chip = MESH_BUF_WAIT2CHIP,
    kChipInUse = MESH_BUF_CHIPINUSE,
};

struct LutBufferConfig {
    bool IsBigMode;
    uint32_t Width;
    uint32_t Height;
    uint32_t LutHCount;
    uint32_t LutVCount;
    uint8_t ScaleFactor;
    uint16_t PsfCfgCount;
};

static const uint32_t IspBigModeWidthLimit = 2688;
static const uint32_t IspBigModeSizeLimit  = IspBigModeWidthLimit * 1536;
static const uint8_t CacScaleFactorDefault = 64;
static const uint8_t CacScaleFactorBigMode = 128;
static const uint8_t CacStrengthDistance   = 128;
static const uint8_t CacPsfKernelSize      = 7 * 5;
static const uint8_t CacPsfKernelWordSizeInMemory =
    DIV_ROUND_UP((CacPsfKernelSize - 1) * BITS_PER_BYTE, BITS_PER_WORD);
static const uint8_t CacChannelCount   = 1;
static const uint32_t CacPsfCountLimit = 1632;

static const uint8_t CacPsfBufferCount = ISP2X_MESH_BUF_NUM;

static inline bool IsIspBigMode(uint32_t width, uint32_t height, bool is_multi_sensor) {
    if (is_multi_sensor || width > IspBigModeWidthLimit || width * height > IspBigModeSizeLimit)
        return true;
    else
        return false;
};

/**
 * cac_wsize=bigmode_en ? (pic_width+126)>>7 : (pic_width+62)>>6;
 * cac_hsize=bigmode_en ? (pic_height+126)>>7 : (pic_height+62)>>6;
 * lut_h_wsize = cac_wsize*9
 * lut_v_size = cac_hsize*2
 */
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig& config) {
    config.Width     = width;
    config.Height    = height;
    config.IsBigMode = is_big_mode;
    if (config.IsBigMode) {
        config.ScaleFactor = CacScaleFactorBigMode;
    } else {
        config.ScaleFactor = CacScaleFactorDefault;
    }
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config.LutHCount   = is_big_mode ? (width + 126) >> 7 : (width + 62) >> 6;
    config.LutVCount   = is_big_mode ? (height + 126) >> 7 : (height + 62) >> 6;
    config.PsfCfgCount = config.LutHCount * config.LutVCount;
    XCAM_ASSERT(config.PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
    // config.LutHWordSize *= CacPsfKernelWordSizeInMemory;
    /**
     * CAC processes R & B channels.
     */
    // config.LutVSize *= CacChannelCount;
}

struct LutBuffer {
    LutBuffer() = delete;
    LutBuffer(const LutBufferConfig& config)
        : State(LutBufferState::kInitial), Config(config), Fd(-1), Size(0), Addr(0) {}
    LutBuffer(const LutBufferConfig& config, const rk_aiq_cac_share_mem_info_t* mem_info)
        : Config(config) {
        State = static_cast<LutBufferState>(*mem_info->state);
        Fd    = mem_info->fd;
        Addr  = mem_info->addr;
        Size  = mem_info->size;
    }
    LutBuffer(const LutBuffer&) = delete;
    LutBuffer& operator=(const LutBuffer&) = delete;

    LutBufferState State;
    LutBufferConfig Config;
    int Fd;
    int Size;
    void* Addr;
};

class LutBufferManager {
 public:
    LutBufferManager() = delete;
    LutBufferManager(const LutBufferConfig& config, const isp_drv_share_mem_ops_t* mem_ops)
        : config_(config), mem_ops_(mem_ops) {}
    LutBufferManager(const LutBufferManager&) = delete;
    LutBufferManager& operator=(const LutBufferManager&) = delete;
    ~LutBufferManager() {
        // TODO(Cody)
        ReleaseHwBuffers(0);
        ReleaseHwBuffers(1);
    }

    void ImportHwBuffers(uint8_t isp_id) {
        assert(mem_ops_ != nullptr);
        rk_aiq_share_mem_config_t hw_config_;
        hw_config_.mem_type           = MEM_TYPE_CAC;
        hw_config_.alloc_param.width  = config_.Width;
        hw_config_.alloc_param.height = config_.Height;

        mem_ops_->alloc_mem(isp_id, (void*)mem_ops_, &hw_config_, &mem_ctx_);
    }

    void ReleaseHwBuffers(uint8_t isp_id) {
        if (mem_ctx_ && mem_ops_) mem_ops_->release_mem(isp_id, mem_ctx_);
    }

    LutBuffer* GetFreeHwBuffer(uint8_t isp_id) {
        if (mem_ops_ == nullptr || mem_ctx_ == nullptr) {
            return nullptr;
        }

        const auto* mem_info = reinterpret_cast<rk_aiq_cac_share_mem_info_t*>(
            mem_ops_->get_free_item(isp_id, mem_ctx_));
        if (mem_info != nullptr) {
            return new LutBuffer(config_, mem_info);
        }
        return nullptr;
    }

 private:
    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    LutBufferConfig config_;
};

CacAlgoAdaptor::~CacAlgoAdaptor() {
    if (current_lut_[0]) delete current_lut_[0];
    if (current_lut_[1]) delete current_lut_[1];
    if (lut_manger_ != nullptr) delete lut_manger_;
}

XCamReturn CacAlgoAdaptor::Config(const AlgoCtxInstanceCfgInt* config,
                                  const CalibDbV2_Cac_t* calib) {
    LOGD_ACAC("%s : Enter", __func__);
    enable_ = calib->enable;
    calib_  = calib;
    if (!enable_) {
        return XCAM_RETURN_BYPASS;
    }

    if (access(calib->Params.psf_path, O_RDONLY)) {
        LOGE_ACAC("The PSF file path %s cannot be accessed", calib->Params.psf_path);
        valid_ = false;
        return XCAM_RETURN_ERROR_FILE;
    }
    valid_ = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn CacAlgoAdaptor::Prepare(const RkAiqAlgoConfigAcacInt* config) {
    LutBufferConfig lut_config;
    LutBufferConfig full_lut_config;
    uint32_t width   = config->width;
    uint32_t height  = config->height;
    bool is_big_mode = IsIspBigMode(width, height, config->is_multi_sensor);

    LOGD_ACAC("%s : Enter", __func__);

    if (!enable_) {
        return XCAM_RETURN_BYPASS;
    }

    if (config->is_multi_isp) {
        CalcCacLutConfig(width, height, is_big_mode, full_lut_config);
        width = width / 2 + config->multi_isp_extended_pixel;
        CalcCacLutConfig(width, height, is_big_mode, lut_config);
    } else {
        CalcCacLutConfig(width, height, is_big_mode, lut_config);
    }
    lut_manger_ = new LutBufferManager(lut_config, config->mem_ops);
    lut_manger_->ImportHwBuffers(0);
    current_lut_[0] = lut_manger_->GetFreeHwBuffer(0);
    XCAM_ASSERT(current_lut_[0] != nullptr);
    if (config->is_multi_isp) {
        lut_manger_->ImportHwBuffers(1);
        current_lut_[1] = lut_manger_->GetFreeHwBuffer(1);
        XCAM_ASSERT(current_lut_[1] != nullptr);
    }

    std::ifstream ifs(calib_->Params.psf_path, std::ios::binary);
    if (!ifs.is_open()) {
        LOGE_ACAC("Failed to open PSF file %s", calib_->Params.psf_path);
        XCAM_ASSERT(0);
        return XCAM_RETURN_ERROR_FILE;
    } else {
        // TODO(Cody) CAC PSF LUT binary only have one channel by now
        if (!config->is_multi_isp) {
            // B channel
            ifs.read(reinterpret_cast<char*>(current_lut_[0]->Addr),
                     lut_config.PsfCfgCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD);
        } else {
            // Read and Split Memory
            //   a == line_size - line_offset
            //   b == line_offset
            //   c == line_offset - a = 2 * line_offset - line_size
            // For each line:
            //   read b size to left
            //   copy c from left to right
            //   read a' to right
            // - +---------------------------+
            // | |<---a---->|  |  |<---a'--->|
            // | |          |<-c->|          |
            // v |<---b---------->|          |
            // | |          |  |  |          |
            // - +---------------------------+
            //   |<---------line_size------->|
            //
            uint32_t line_offset =
                lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
            uint32_t line_size =
                full_lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
            for (int ch = 0; ch < CacChannelCount; ch++) {
                for (uint32_t i = 0; i < full_lut_config.LutVCount; i++) {
                    ifs.read(reinterpret_cast<char*>(current_lut_[0]->Addr) + (i * line_offset),
                             line_offset);
                    memcpy(reinterpret_cast<char*>(current_lut_[1]->Addr) + (i * line_offset),
                           reinterpret_cast<char*>(current_lut_[0]->Addr) + (i * line_offset) +
                               line_size - line_offset,
                           2 * line_offset - line_size);
                    ifs.read(reinterpret_cast<char*>(current_lut_[1]->Addr) + (i * line_size) +
                                 line_offset,
                             line_size - line_offset);
                }
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

void CacAlgoAdaptor::OnFrameEvent(const RkAiqAlgoProcAcacInt* input,
                                  RkAiqAlgoProcResAcacInt* output) {
    int i;
    int iso_low  = 50;
    int iso_high = 50;
    int gain_high, gain_low;
    float ratio;
    int iso_div      = 50;
    int max_iso_step = calib_->Params.iso_len;
    int iso          = input->iso;
    LOGD_ACAC("%s : Enter", __func__);

    if (!enable_ || !valid_) {
        // TODO(Cody)
        output->config[0].bypass_en = 1;
        output->config[1].bypass_en = 1;
        LOGD_ACAC("%s : bypassed enable:%d valid:%d", __func__, enable_, valid_);
        return;
    }

    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    gain_high = (int)(log((float)iso_high / 50) / log((float)2));
    gain_low  = (int)(log((float)iso_low / 50) / log((float)2));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);

    XCAM_ASSERT(gain_low >= 0 && gain_low < max_iso_step);
    XCAM_ASSERT(gain_high >= 0 && gain_high < max_iso_step);

    for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        output->config[0].strength[i] =
            INTERP_CAC(calib_->Params.strength_table[gain_low][i],
                       calib_->Params.strength_table[gain_high][i], ratio);
    }
    output->config[0].bypass_en =
        INTERP_CAC(calib_->Params.bypass[gain_low], calib_->Params.bypass[gain_high], ratio);
    output->config[0].center_en     = calib_->Params.center_en;
    output->config[0].center_width  = calib_->Params.center_x;
    output->config[0].center_height = calib_->Params.center_y;
    output->config[0].psf_sft_bit   = calib_->Params.psf_shift_bits;
    output->config[0].cfg_num       = current_lut_[0]->Config.PsfCfgCount;
    output->config[0].buf_fd        = current_lut_[0]->Fd;
    output->config[0].hsize         = current_lut_[0]->Config.LutHCount * (CacPsfKernelSize - 1);
    output->config[0].vsize         = current_lut_[0]->Config.LutVCount * CacChannelCount;
    if (current_lut_[1]) {
        memcpy(&output->config[1], &output->config[0], sizeof(output->config[0]));
        output->config[1].buf_fd = current_lut_[1]->Fd;
    }

    LOGD_ACAC("global en : %d", calib_->enable);
    LOGD_ACAC("global bypass: %d", calib_->bypass);
    LOGD_ACAC("current bypass: %d", output->config[0].bypass_en);
    LOGD_ACAC("center en: %d", output->config[0].center_en);
    LOGD_ACAC("center x: %u", output->config[0].center_width);
    LOGD_ACAC("center y: %u", output->config[0].center_height);
    LOGD_ACAC("psf shift bits: %u", output->config[0].psf_sft_bit);
    LOGD_ACAC("psf cfg num: %u", output->config[0].cfg_num);
    LOGD_ACAC("psf buf fd: %d", output->config[0].buf_fd);
    if (current_lut_[1]) {
        LOGD_ACAC("psf buf fd right: %d", output->config[1].buf_fd);
    }
    LOGD_ACAC("psf hwsize: %u", output->config[0].hsize);
    LOGD_ACAC("psf size: %u", output->config[0].vsize);
    for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        LOGD_ACAC("strength %d: %u", i, output->config[0].strength[i]);
    }
}

}  // namespace RkCam

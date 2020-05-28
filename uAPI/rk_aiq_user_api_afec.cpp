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

#include "rk_aiq_user_api_afec.h"
#include "RkAiqHandleInt.h"

RKAIQ_BEGIN_DECLARE

XCamReturn
rk_aiq_user_api_afec_enable(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RkAiqAfecHandleInt* algo_handle =
        algoHandle<RkAiqAfecHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AFEC);

    if (algo_handle) {
        return algo_handle->enable();
    }

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn
rk_aiq_user_api_afec_disable(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RkAiqAfecHandleInt* algo_handle =
        algoHandle<RkAiqAfecHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AFEC);

    if (algo_handle) {
        return algo_handle->disable();
    }

    return XCAM_RETURN_ERROR_FAILED;
}

RKAIQ_END_DECLARE

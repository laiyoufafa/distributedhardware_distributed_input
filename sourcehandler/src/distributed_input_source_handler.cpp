/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "distributed_input_source_handler.h"

#include "dinput_errcode.h"
#include "dinput_log.h"
#include "hisysevent_util.h"
#include "i_distributed_source_input.h"
#include "load_d_input_source_callback.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
IMPLEMENT_SINGLE_INSTANCE(DistributedInputSourceHandler);

DistributedInputSourceHandler::~DistributedInputSourceHandler()
{
}

int32_t DistributedInputSourceHandler::InitSource(const std::string &params)
{
    DHLOGD("InitSource");
    std::unique_lock<std::mutex> lock(proxyMutex_);
    if (!DInputSAManager::GetInstance().HasDInputSourceProxy()) {
        sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!samgr) {
            DHLOGE("Failed to get system ability mgr.");
            return ERR_DH_INPUT_SINK_HANDLER_INIT_SOURCE_SA_FAIL;
        }
        sptr<LoadDInputSourceCallback> loadCallback = new LoadDInputSourceCallback(params);
        HisyseventUtil::GetInstance().SysEventWriteBehavior(DINPUT_INIT,
            "dinput init source sa start.");
        int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, loadCallback);
        if (ret != ERR_OK) {
            DHLOGE("Failed to Load systemAbility, systemAbilityId:%d, ret code:%d",
                   DISTRIBUTED_HARDWARE_INPUT_SOURCE_SA_ID, ret);
            return ERR_DH_INPUT_SINK_HANDLER_INIT_SOURCE_SA_FAIL;
        }
    }

    auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(INPUT_LOAD_SA_TIMEOUT_MS),
        [this]() { return (DInputSAManager::GetInstance().HasDInputSourceProxy()); });
    if (!waitStatus) {
        DHLOGE("dinput load sa timeout.");
        return ERR_DH_INPUT_SINK_HANDLER_INIT_SOURCE_SA_FAIL;
    }

    return DH_SUCCESS;
}

void DistributedInputSourceHandler::FinishStartSA(const std::string &params, const sptr<IRemoteObject> &remoteObject)
{
    DHLOGD("FinishStartSA");
    std::unique_lock<std::mutex> lock(proxyMutex_);
    DInputSAManager::GetInstance().SetDInputSourceProxy(remoteObject);
    DistributedInputClient::GetInstance().InitSource();
    proxyConVar_.notify_all();
}

int32_t DistributedInputSourceHandler::ReleaseSource()
{
    return DistributedInputClient::GetInstance().ReleaseSource();
}

int32_t DistributedInputSourceHandler::RegisterDistributedHardware(const std::string &devId,
    const std::string &dhId, const EnableParam &param, std::shared_ptr<RegisterCallback> callback)
{
    return DistributedInputClient::GetInstance().RegisterDistributedHardware(devId, dhId, param.attrs, callback);
}

int32_t DistributedInputSourceHandler::UnregisterDistributedHardware(const std::string &devId,
    const std::string &dhId, std::shared_ptr<UnregisterCallback> callback)
{
    return DistributedInputClient::GetInstance().UnregisterDistributedHardware(devId, dhId, callback);
}

int32_t DistributedInputSourceHandler::ConfigDistributedHardware(const std::string &devId,
    const std::string &dhId, const std::string &key, const std::string &value)
{
    return DH_SUCCESS;
}

void DistributedInputSourceHandler::SALoadSourceCb::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const OHOS::sptr<IRemoteObject>& remoteObject)
{
    currSystemAbilityId = systemAbilityId;
    currRemoteObject = remoteObject;
    DHLOGI("DistributedInputSourceHandler OnLoadSystemAbilitySuccess. systemAbilityId=%d", systemAbilityId);
}

void DistributedInputSourceHandler::SALoadSourceCb::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    currSystemAbilityId = systemAbilityId;
    DHLOGE("DistributedInputSourceHandler OnLoadSystemAbilityFail. systemAbilityId=%d", systemAbilityId);
}

IDistributedHardwareSource *GetSourceHardwareHandler()
{
    return &DistributedInputSourceHandler::GetInstance();
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

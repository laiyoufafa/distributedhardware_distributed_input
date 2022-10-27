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

#include "distributed_input_collector.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <securec.h>
#include <unistd.h>

#include <openssl/sha.h>
#include <sys/inotify.h>
#include <linux/input.h>

#include "anonymous_string.h"
#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"

#include "dinput_errcode.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
DistributedInputCollector::DistributedInputCollector() : mEventBuffer{}, collectThreadID_(-1),
    isCollectingEvents_(false), isStartGetDeviceHandlerThread(false), inputTypes_(0)
{
    inputHub_ = std::make_unique<InputHub>();
}

DistributedInputCollector::~DistributedInputCollector()
{
    StopCollectEventsThread();
}

DistributedInputCollector &DistributedInputCollector::GetInstance()
{
    static DistributedInputCollector instance;
    return instance;
}

int32_t DistributedInputCollector::Init(std::shared_ptr<AppExecFwk::EventHandler> sinkHandler)
{
    sinkHandler_ = sinkHandler;
    if (sinkHandler_ == nullptr) {
        DHLOGE("DistributedInputCollector::Init sinkHandler_ failed \n");
        return ERR_DH_INPUT_SERVER_SINK_COLLECTOR_INIT_FAIL;
    }
    if (!isStartGetDeviceHandlerThread) {
        InitCollectEventsThread();
        isStartGetDeviceHandlerThread = true;
    }
    return DH_SUCCESS;
}

bool DistributedInputCollector::InitCollectEventsThread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    isCollectingEvents_ = true;
    collectThreadID_ = -1;
    int32_t ret = pthread_create(&collectThreadID_, &attr, CollectEventsThread, this);
    if (ret != 0) {
        DHLOGE("DistributedInputCollector::InitCollectEventsThread create thread failed:%d \n", ret);
        pthread_attr_destroy(&attr);
        collectThreadID_ = -1;
        isCollectingEvents_ = false;
        return false;
    }
    return true;
}

void *DistributedInputCollector::CollectEventsThread(void *param)
{
    DistributedInputCollector *pThis = reinterpret_cast<DistributedInputCollector *>(param);
    pThis->StartCollectEventsThread();
    DHLOGW("DistributedInputCollector::CollectEventsThread exist!");
    return nullptr;
}

void DistributedInputCollector::StartCollectEventsThread()
{
    while (isCollectingEvents_) {
        memset_s(&mEventBuffer, sizeof(mEventBuffer), 0, sizeof(mEventBuffer));
        if (inputHub_ == nullptr) {
            DHLOGI("inputHub is nullptr!");
            return;
        }
        size_t count = inputHub_->StartCollectInputEvents(mEventBuffer, INPUT_EVENT_BUFFER_SIZE);
        if (count > 0) {
            DHLOGI("Count: %zu", count);
        } else {
            continue;
        }

        // The RawEvent obtained by the controlled end calls transport and is
        // sent to the main control end.
        std::shared_ptr<nlohmann::json> jsonArrayMsg = std::make_shared<nlohmann::json>();
        for (size_t ind = 0; ind < count; ind++) {
            nlohmann::json tmpJson;
            tmpJson[INPUT_KEY_WHEN] = mEventBuffer[ind].when;
            tmpJson[INPUT_KEY_TYPE] = mEventBuffer[ind].type;
            tmpJson[INPUT_KEY_CODE] = mEventBuffer[ind].code;
            tmpJson[INPUT_KEY_VALUE] = mEventBuffer[ind].value;
            tmpJson[INPUT_KEY_PATH] = mEventBuffer[ind].path;
            tmpJson[INPUT_KEY_DESCRIPTOR] = mEventBuffer[ind].descriptor;
            jsonArrayMsg->push_back(tmpJson);
        }

        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(
            static_cast<uint32_t>(EHandlerMsgType::DINPUT_SINK_EVENT_HANDLER_MSG), jsonArrayMsg, 0);
        if (sinkHandler_ != nullptr) {
            sinkHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    }
    DHLOGW("DistributedInputCollector::StartCollectEventsThread exit!");
}

void DistributedInputCollector::StopCollectEventsThread()
{
    isCollectingEvents_ = false;
    isStartGetDeviceHandlerThread = false;
    if (inputHub_ == nullptr) {
        DHLOGI("inputHub is nullptr!");
        return;
    }
    inputHub_->StopCollectInputEvents();
    if (collectThreadID_ != (pthread_t)(-1)) {
        DHLOGI("DistributedInputCollector::Wait collect thread exit");
        pthread_join(collectThreadID_, NULL);
        collectThreadID_ = (pthread_t)(-1);
    }
    DHLOGW("DistributedInputCollector::StopCollectEventsThread exit!");
}

AffectDhIds DistributedInputCollector::SetSharingTypes(bool enabled, const uint32_t &inputType)
{
    inputTypes_ = 0;
    if ((inputType & static_cast<uint32_t>(DInputDeviceType::MOUSE)) != 0 ||
        (inputType & static_cast<uint32_t>(DInputDeviceType::TOUCHPAD)) != 0) {
        inputTypes_ |= INPUT_DEVICE_CLASS_CURSOR;
    }
    if ((inputType & static_cast<uint32_t>(DInputDeviceType::KEYBOARD)) != 0) {
        inputTypes_ |= INPUT_DEVICE_CLASS_KEYBOARD;
    }
    if ((inputType & static_cast<uint32_t>(DInputDeviceType::TOUCHSCREEN)) != 0) {
        inputTypes_ |= INPUT_DEVICE_CLASS_TOUCH_MT | INPUT_DEVICE_CLASS_TOUCH;
    }

    return inputHub_->SetSupportInputType(enabled, inputTypes_);
}

void DistributedInputCollector::ReportDhIdSharingState(const AffectDhIds &dhIds)
{
    std::lock_guard<std::mutex> lock(sharingDhIdListenerMtx_);
    if (sharingDhIdListeners_.size() == 0) {
        DHLOGI("sharingDhIdListeners_ is null, can not report sharing dhid");
        return;
    }

    for (auto const &id : dhIds.sharingDhIds) {
        DHLOGI("Sharing DhId: %s", GetAnonyString(id).c_str());
        for (auto iter : sharingDhIdListeners_) {
            iter->OnSharing(id);
        }
    }

    for (auto const &id : dhIds.noSharingDhIds) {
        DHLOGI("No Sharing DhId: %s", GetAnonyString(id).c_str());
        for (auto iter : sharingDhIdListeners_) {
            iter->OnNoSharing(id);
        }
    }
}

void DistributedInputCollector::Release()
{
    StopCollectEventsThread();
}

AffectDhIds DistributedInputCollector::SetSharingDhIds(bool enabled, std::vector<std::string> dhIds)
{
    return inputHub_->SetSharingDevices(enabled, dhIds);
}

void DistributedInputCollector::GetMouseNodePath(
    std::vector<std::string> dhIds, std::string &mouseNodePath, std::string &dhid)
{
    if (inputHub_ == nullptr) {
        DHLOGI("inputHub is nullptr!");
        return;
    }
    inputHub_->GetShareMousePathByDhId(dhIds, mouseNodePath, dhid);
}

bool DistributedInputCollector::IsAllDevicesStoped()
{
    if (inputHub_ == nullptr) {
        DHLOGI("inputHub is nullptr!");
        return false;
    }
    return inputHub_->IsAllDevicesStoped();
}

int32_t DistributedInputCollector::RegisterSharingDhIdListener(sptr<ISharingDhIdListener> sharingDhIdListener)
{
    DHLOGI("RegisterSharingDhIdListener");
    std::lock_guard<std::mutex> lock(sharingDhIdListenerMtx_);
    sharingDhIdListeners_.insert(sharingDhIdListener);
    return DH_SUCCESS;
}

void DistributedInputCollector::GetDeviceInfoByType(const uint32_t inputTypes, std::map<int32_t,
    std::string>& deviceInfo)
{
    if (inputHub_ == nullptr) {
        DHLOGI("inputHub is nullptr!");
        return;
    }
    inputHub_->GetDevicesInfoByType(inputTypes, deviceInfo);
}
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

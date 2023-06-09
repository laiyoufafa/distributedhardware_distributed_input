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

#ifndef DISTRIBUTED_INPUT_NODE_MANAGER_H
#define DISTRIBUTED_INPUT_NODE_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "nlohmann/json.hpp"

#include "constants_dinput.h"
#include "input_hub.h"
#include "virtual_device.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DistributedInputNodeManager {
public:
    DistributedInputNodeManager();
    ~DistributedInputNodeManager();

    int32_t openDevicesNode(const std::string& devId, const std::string& dhId,
    const std::string& parameters);

    int32_t getDevice(const std::string& dhId, VirtualDevice*& device);
    void ReportEvent(const RawEvent rawEvent);
    int32_t CloseDeviceLocked(const std::string& dhId);
    void StartInjectThread();
    void StopInjectThread();
    int32_t CreateVirtualTouchScreenNode(const std::string& devId, const std::string& dhId, const uint64_t srcWinId,
        const uint32_t sourcePhyWidth, const uint32_t sourcePhyHeight);
    int32_t RemoveVirtualTouchScreenNode(const std::string& dhId);
    int32_t GetVirtualTouchScreenFd();

    int32_t GetDeviceInfo(std::string &deviceId);
    void GetDevicesInfoByType(const std::string &networkId, uint32_t inputTypes, std::map<int32_t, std::string> &datas);
    void GetDevicesInfoByDhId(std::vector<std::string> dhidsVec, std::map<int32_t, std::string> &datas);
    void ProcessInjectEvent(const std::shared_ptr<RawEvent> &rawEvent);

private:
    void AddDeviceLocked(const std::string& dhId, std::unique_ptr<VirtualDevice> device);
    int32_t CreateHandle(const InputDevice& inputDevice, const std::string& devId, const std::string& dhId);
    void ParseInputDeviceJson(const std::string& str, InputDevice& pBuf);
    void VerifyInputDevice(const nlohmann::json& inputDeviceJson, InputDevice& pBuf);
    void InjectEvent();

    /* the key is dhId, and the value is virtualDevice */
    std::map<std::string, std::unique_ptr<VirtualDevice>> virtualDeviceMap_;
    std::mutex virtualDeviceMapMutex_;
    std::atomic<bool> isInjectThreadCreated_;
    std::atomic<bool> isInjectThreadRunning_;
    std::mutex operationMutex_;
    std::thread eventInjectThread_;
    std::mutex injectThreadMutex_;
    std::condition_variable conditionVariable_;
    std::queue<std::shared_ptr<RawEvent>> injectQueue_;
    std::unique_ptr<InputHub> inputHub_;
    int32_t virtualTouchScreenFd_;
    std::once_flag callOnceFlag_;
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_INJECT_H
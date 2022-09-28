/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INPUT_CHECK_PARAM_H
#define INPUT_CHECK_PARAM_H

#include <iremote_broker.h>

#include "idistributed_hardware_source.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class DinputCheckParam {
public:
    static DinputCheckParam &GetInstance(void);
    bool CheckParam(const std::string &deviceId, sptr<IRemoteBroker> callback);
    bool CheckParam(const std::string &deviceId, const uint32_t &inputTypes, sptr<IRemoteBroker> callback);
    bool CheckParam(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IRemoteBroker> callback);
    bool CheckParam(const std::string &srcId, const std::string &sinkId, sptr<IRemoteBroker> callback);
    bool CheckParam(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IRemoteBroker> callback);
    bool CheckParam(const std::string &srcId, const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IRemoteBroker> callback);
    bool CheckRegisterParam(const std::string &devId, const std::string &dhId,
        const std::string &parameters, const std::shared_ptr<RegisterCallback> &callback);
    bool CheckUnregisterParam(const std::string &devId, const std::string &dhId,
        const std::shared_ptr<UnregisterCallback> &callback);

private:
    DinputCheckParam() = default;
    ~DinputCheckParam() = default;
};

} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // INPUT_CHECK_PARAM_H
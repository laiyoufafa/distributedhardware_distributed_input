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

#ifndef UNPREPARE_D_INPUT_CALL_BACK_STUB_H
#define UNPREPARE_D_INPUT_CALL_BACK_STUB_H

#include "i_unprepare_d_input_call_back.h"

#include <string>

#include "iremote_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
class UnprepareDInputCallbackStub : public IRemoteStub<IUnprepareDInputCallback> {
public:
    UnprepareDInputCallbackStub();
    ~UnprepareDInputCallbackStub() override;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(UnprepareDInputCallbackStub);
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // UNPREPARE_D_INPUT_CALL_BACK_STUB_H

/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "distributed_input_source_transport_fuzzer.h"
#include "distributed_input_source_transport.h"

#include <cstddef>
#include <cstdint>

#include "constants_dinput.h"

#include <thread>
#include <functional>
#include <iostream>
#include <refbase.h>

namespace OHOS {
namespace DistributedHardware {
void OpenInputSoftbusFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    std::string remoteDevId(reinterpret_cast<const char*>(data), size);
    std::string hwId(reinterpret_cast<const char*>(data), size);

    DistributedInput::DistributedInputSourceTransport::GetInstance().OpenInputSoftbus(remoteDevId, hwId);
    DistributedInput::DistributedInputSourceTransport::GetInstance().CloseInputSoftbus(remoteDevId, hwId);
}

void OnSessionOpenedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    int32_t result = *(reinterpret_cast<const int32_t*>(data));

    DistributedInput::DistributedInputSourceTransport::GetInstance().OnSessionOpened(sessionId, result);
    DistributedInput::DistributedInputSourceTransport::GetInstance().OnSessionClosed(sessionId);
}

void OnBytesReceivedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    const char *msg = reinterpret_cast<const char *>(data);
    uint16_t dataLen = *(reinterpret_cast<const uint16_t*>(data));
    DistributedInput::DistributedInputSourceTransport::GetInstance().OnBytesReceived(sessionId, msg, dataLen);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OpenInputSoftbusFuzzTest(data, size);
    OHOS::DistributedHardware::OnSessionOpenedFuzzTest(data, size);
    OHOS::DistributedHardware::OnBytesReceivedFuzzTest(data, size);
    return 0;
}
/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H
#define DISTRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H

#include <cstring>
#include <mutex>
#include <set>

#include <unistd.h>
#include <sys/types.h>

#include "event_handler.h"
#include "ipublisher_listener.h"
#include "publisher_listener_stub.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_status_change_stub.h"

#include "constants_dinput.h"
#include "dinput_context.h"
#include "dinput_source_trans_callback.h"
#include "distributed_input_node_manager.h"
#include "distributed_input_source_event_handler.h"
#include "distributed_input_source_sa_cli_mgr.h"
#include "distributed_input_source_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace DistributedInput {
using EventRunner = OHOS::AppExecFwk::EventRunner;
using EventHandler = OHOS::AppExecFwk::EventHandler;
enum class ServiceSourceRunningState { STATE_NOT_START, STATE_RUNNING };
const uint32_t DINPUT_SOURCE_MANAGER_RIGISTER_MSG = 1;
const uint32_t DINPUT_SOURCE_MANAGER_UNRIGISTER_MSG = 2;
const uint32_t DINPUT_SOURCE_MANAGER_PREPARE_MSG = 3;
const uint32_t DINPUT_SOURCE_MANAGER_UNPREPARE_MSG = 4;
const uint32_t DINPUT_SOURCE_MANAGER_START_MSG = 5;
const uint32_t DINPUT_SOURCE_MANAGER_STOP_MSG = 6;
const uint32_t DINPUT_SOURCE_MANAGER_START_DHID_MSG = 7;
const uint32_t DINPUT_SOURCE_MANAGER_STOP_DHID_MSG = 8;
const uint32_t DINPUT_SOURCE_MANAGER_RECEIVE_DATA_MSG = 9;
const uint32_t DINPUT_SOURCE_MANAGER_STARTSERVER_MSG = 10;
const uint32_t DINPUT_SOURCE_MANAGER_KEY_STATE_MSG = 11;

const uint32_t DINPUT_SOURCE_MANAGER_RELAY_PREPARE_RESULT_TO_ORIGIN   = 12;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_UNPREPARE_RESULT_TO_ORIGIN = 13;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_PREPARE_RESULT_MMI   = 14;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_UNPREPARE_RESULT_MMI = 15;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_STARTDHID_RESULT_MMI = 16;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_STOPDHID_RESULT_MMI  = 17;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_STARTTYPE_RESULT_MMI = 18;
const uint32_t DINPUT_SOURCE_MANAGER_RELAY_STOPTYPE_RESULT_MMI  = 19;

const std::string INPUT_SOURCEMANAGER_KEY_SESSIONID = "sessionId";
const std::string INPUT_SOURCEMANAGER_KEY_DEVID = "deviceId";
const std::string INPUT_SOURCEMANAGER_KEY_HWID = "hardwareId";
const std::string INPUT_SOURCEMANAGER_KEY_ITP = "inputTypes";
const std::string INPUT_SOURCEMANAGER_KEY_DHID = "dhids";
const std::string INPUT_SOURCEMANAGER_KEY_TYPE = "type";
const std::string INPUT_SOURCEMANAGER_KEY_CODE = "code";
const std::string INPUT_SOURCEMANAGER_KEY_VALUE = "value";
const std::string INPUT_SOURCEMANAGER_KEY_RESULT = "result";
const std::string INPUT_SOURCEMANAGER_KEY_WHITELIST = "whitelist";
const std::string INPUT_SOURCEMANAGER_KEY_SRC_DEVID = "srcId";
const std::string INPUT_SOURCEMANAGER_KEY_SINK_DEVID = "sinkId";

const uint32_t DINPUT_SOURCE_SWITCH_OFF = 0;
const uint32_t DINPUT_SOURCE_SWITCH_ON = 1;
const uint32_t DINPUT_SOURCE_WRITE_EVENT_SIZE = 1;

// Node Info that registerd by remote node
typedef struct BeRegNodeInfo {
    // source node network id
    std::string srcId;
    // sink node dh id
    std::string dhId;
    // node desc on sink node
    std::string nodeDesc;

    bool operator==(const BeRegNodeInfo &node)
    {
        return (srcId == node.srcId) && (dhId == node.dhId) && (nodeDesc == node.nodeDesc);
    }

    bool operator<(const BeRegNodeInfo &node) const
    {
        return (srcId + dhId + nodeDesc).compare(node.srcId + node.dhId + node.nodeDesc) < 0;
    }
} BeRegNodeInfo;

class DistributedInputSourceManager : public SystemAbility, public DistributedInputSourceStub {
    DECLARE_SYSTEM_ABILITY(DistributedInputSourceManager)

typedef struct InputDeviceId {
    std::string devId;
    std::string dhId;
    std::string nodeDesc;

    bool operator==(const InputDeviceId &inputId)
    {
        return (devId == inputId.devId) && (dhId == inputId.dhId) && (nodeDesc == inputId.nodeDesc);
    }
} InputDeviceId;

public:
    DistributedInputSourceManager(int32_t saId, bool runOnCreate);
    ~DistributedInputSourceManager() override;

    void OnStart() override;

    void OnStop() override;

    int32_t Init() override;

    int32_t Release() override;

    int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& parameters, sptr<IRegisterDInputCallback> callback) override;

    int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        sptr<IUnregisterDInputCallback> callback) override;

    int32_t PrepareRemoteInput(const std::string &deviceId, sptr<IPrepareDInputCallback> callback) override;

    int32_t UnprepareRemoteInput(const std::string &deviceId, sptr<IUnprepareDInputCallback> callback) override;

    int32_t StartRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStartDInputCallback> callback) override;

    int32_t StopRemoteInput(
        const std::string& deviceId, const uint32_t& inputTypes, sptr<IStopDInputCallback> callback) override;

    int32_t StartRemoteInput(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStartDInputCallback> callback) override;

    int32_t StopRemoteInput(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStopDInputCallback> callback) override;

    int32_t PrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IPrepareDInputCallback> callback) override;

    int32_t UnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IUnprepareDInputCallback> callback) override;

    int32_t StartRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IStartStopDInputsCallback> callback) override;

    int32_t StopRemoteInput(const std::string &sinkId, const std::vector<std::string> &dhIds,
        sptr<IStartStopDInputsCallback> callback) override;

    int32_t StartRemoteInput(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback) override;

    int32_t StopRemoteInput(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback) override;

    int32_t RegisterAddWhiteListCallback(sptr<IAddWhiteListInfosCallback> addWhiteListCallback) override;
    int32_t RegisterDelWhiteListCallback(sptr<IDelWhiteListInfosCallback> delWhiteListCallback) override;
    int32_t RegisterInputNodeListener(sptr<InputNodeListener> listener) override;
    int32_t UnregisterInputNodeListener(sptr<InputNodeListener> listener) override;

    int32_t SyncNodeInfoRemoteInput(const std::string &userDevId, const std::string &dhId,
        const std::string &nodeDesc) override;
    int32_t RegisterSimulationEventListener(sptr<ISimulationEventListener> listener) override;
    int32_t UnregisterSimulationEventListener(sptr<ISimulationEventListener> listener) override;
    bool CheckRegisterParam(const std::string &devId, const std::string &dhId,
        const std::string &parameters, sptr<IRegisterDInputCallback> callback);
    bool CheckUnregisterParam(const std::string &devId, const std::string &dhId,
        sptr<IUnregisterDInputCallback> callback);
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    class DInputSourceListener : public DInputSourceTransCallback {
    public:
        explicit DInputSourceListener(DistributedInputSourceManager *manager);
        virtual ~DInputSourceListener();
        void OnResponseRegisterDistributedHardware(const std::string deviceId, const std::string dhId,
            bool result) override;
        void OnResponsePrepareRemoteInput(const std::string deviceId, bool result, const std::string &object) override;
        void OnResponseUnprepareRemoteInput(const std::string deviceId, bool result) override;
        void OnResponseStartRemoteInput(const std::string deviceId, const uint32_t inputTypes, bool result) override;
        void OnResponseStopRemoteInput(const std::string deviceId, const uint32_t inputTypes, bool result) override;
        void OnResponseStartRemoteInputDhid(const std::string deviceId, const std::string &dhids, bool result) override;
        void OnResponseStopRemoteInputDhid(const std::string deviceId, const std::string &dhids, bool result) override;
        void OnResponseKeyState(const std::string deviceId, const std::string &dhid, const uint32_t type,
            const uint32_t code, const uint32_t value) override;
        void OnReceivedEventRemoteInput(const std::string deviceId, const std::string &event) override;
        void OnResponseRelayPrepareRemoteInput(int32_t sessionId, const std::string &deviceId, bool result,
            const std::string &object) override;
        void OnResponseRelayUnprepareRemoteInput(int32_t sessionId, const std::string &deviceId, bool result) override;

        void OnReceiveRelayPrepareResult(int32_t status, const std::string &srcId, const std::string &sinkId) override;
        void OnReceiveRelayUnprepareResult(int32_t status, const std::string &srcId,
            const std::string &sinkId) override;
        void OnReceiveRelayStartDhidResult(int32_t status, const std::string &srcId, const std::string &sinkId,
            const std::string &dhids) override;
        void OnReceiveRelayStopDhidResult(int32_t status, const std::string &srcId, const std::string &sinkId,
            const std::string &dhids) override;
        void OnReceiveRelayStartTypeResult(int32_t status, const std::string &srcId, const std::string &sinkId,
            uint32_t inputTypes) override;
        void OnReceiveRelayStopTypeResult(int32_t status, const std::string &srcId, const std::string &sinkId,
            uint32_t inputTypes) override;
        void RecordEventLog(int64_t when, int32_t type, int32_t code, int32_t value, const std::string& path);

    private:
        DistributedInputSourceManager *sourceManagerObj_;
    };

    class DInputSourceManagerEventHandler : public AppExecFwk::EventHandler {
    public:
        DInputSourceManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
            DistributedInputSourceManager *manager);
        ~DInputSourceManagerEventHandler() override;

        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    private:
        void NotifyRegisterCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyUnregisterCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyPrepareCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyUnprepareCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStartCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStopCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStartDhidCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStopDhidCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyKeyStateCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyStartServerCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayPrepareCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayUnprepareCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayStartDhidCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayStopDhidCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayStartTypeCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayStopTypeCallback(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayPrepareRemoteInput(const AppExecFwk::InnerEvent::Pointer &event);
        void NotifyRelayUnprepareRemoteInput(const AppExecFwk::InnerEvent::Pointer &event);

        using SourceEventFunc = void (DInputSourceManagerEventHandler::*)(
            const AppExecFwk::InnerEvent::Pointer &event);
        std::map<int32_t, SourceEventFunc> eventFuncMap_;
        DistributedInputSourceManager *sourceManagerObj_;
    };

    class StartDScreenListener : public PublisherListenerStub {
    public:
        StartDScreenListener();
        ~StartDScreenListener() override;
        void OnMessage(const DHTopic topic, const std::string& message) override;

    private:
        int32_t ParseMessage(const std::string& message, std::string& sinkDevId, SrcScreenInfo& srcScreenInfo);
        int32_t UpdateSrcScreenInfoCache(const SrcScreenInfo& tmpInfo);
    };

    class StopDScreenListener : public PublisherListenerStub {
    public:
        StopDScreenListener();
        ~StopDScreenListener() override;
        void OnMessage(const DHTopic topic, const std::string& message) override;

    private:
        int32_t ParseMessage(const std::string& message, std::string& sinkDevId, uint64_t& sourceWinId);
    };

    class DeviceOfflineListener : public PublisherListenerStub {
    public:
        explicit DeviceOfflineListener(DistributedInputSourceManager* srcManagerContext);
        ~DeviceOfflineListener() override;

        void OnMessage(const DHTopic topic, const std::string& message) override;

    private:
        void DeleteNodeInfoAndNotify(const std::string& offlineDevId);

    private:
        DistributedInputSourceManager* sourceManagerContext_;
    };

    class DScreenSourceSvrRecipient : public IRemoteObject::DeathRecipient {
    public:
        DScreenSourceSvrRecipient(const std::string& srcDevId, const std::string& sinkDevId, const uint64_t srcWinId);
        ~DScreenSourceSvrRecipient() override;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::string srcDevId_;
        std::string sinkDevId_;
        uint64_t srcWinId_;
    };

    std::shared_ptr<DInputSourceManagerEventHandler> GetCallbackEventHandler()
    {
        return callBackHandler_;
    };

public:
    void RunRegisterCallback(const std::string &devId, const std::string &dhId, const int32_t &status);
    void RunUnregisterCallback(const std::string &devId, const std::string &dhId, const int32_t &status);
    void RunPrepareCallback(const std::string &devId, const int32_t &status, const std::string &object);
    void RunUnprepareCallback(const std::string &devId, const int32_t &status);
    void RunStartCallback(const std::string &devId, const uint32_t &inputTypes, const int32_t &status);
    void RunStopCallback(const std::string &devId, const uint32_t &inputTypes, const int32_t &status);

    void RunStartDhidCallback(const std::string &sinkId, const std::string &dhIds, const int32_t &status);
    void RunStopDhidCallback(const std::string &sinkId, const std::string &dhIds, const int32_t &status);
    void RunKeyStateCallback(const std::string &sinkId, const std::string &dhId, const uint32_t type,
        const uint32_t code, const uint32_t value);
    void RunWhiteListCallback(const std::string &devId, const std::string &object);

    DInputServerType GetStartTransFlag();
    void SetStartTransFlag(const DInputServerType flag);
    std::vector<InputDeviceId> GetInputDeviceId();
    void RemoveInputDeviceId(const std::string deviceId, const std::string dhId);
    void SetDeviceMapValue(const std::string deviceId, int32_t value);
    bool GetDeviceMapAllDevSwitchOff();
    int32_t CheckDeviceIsExists(const std::string &devId, const std::string &dhId,
        const InputDeviceId &inputDeviceId, std::vector<InputDeviceId>::iterator &it);
    int32_t DeleteInputDeviceNodeInfo(const std::string &devId, const std::string &dhId,
        const std::vector<InputDeviceId>::iterator &it);
    int32_t UnregCallbackNotify(const std::string &devId, const std::string &dhId);
    void SetInputTypesMap(const std::string deviceId, uint32_t value);
    uint32_t GetInputTypesMap(const std::string deviceId);
    uint32_t GetAllInputTypesMap();

private:
    struct DInputClientRegistInfo {
        std::string devId;
        std::string dhId;
        sptr<IRegisterDInputCallback> callback = nullptr;
    };

    struct DInputClientUnregistInfo {
        std::string devId;
        std::string dhId;
        sptr<IUnregisterDInputCallback> callback = nullptr;
    };

    struct DInputClientPrepareInfo {
        std::string devId;
        sptr<IPrepareDInputCallback> preCallback = nullptr;

        DInputClientPrepareInfo(std::string deviceId, sptr<IPrepareDInputCallback> prepareCallback)
            : devId(deviceId), preCallback(prepareCallback) {}
    };

    struct DInputClientUnprepareInfo {
        std::string devId;
        sptr<IUnprepareDInputCallback> unpreCallback = nullptr;
    };

    struct DInputClientStartInfo {
        std::string devId;
        uint32_t inputTypes;
        sptr<IStartDInputCallback> callback = nullptr;
        DInputClientStartInfo(std::string deviceId, uint32_t types, sptr<IStartDInputCallback> cb)
            : devId(deviceId), inputTypes(types), callback(cb) {}
    };

    struct DInputClientStopInfo {
        std::string devId;
        uint32_t inputTypes;
        sptr<IStopDInputCallback> callback = nullptr;
        DInputClientStopInfo(std::string deviceId, uint32_t types, sptr<IStopDInputCallback> cb)
            : devId(deviceId), inputTypes(types), callback(cb) {}
    };
    // add new prepare/start function
    struct DInputClientRelayPrepareInfo {
        std::string srcId;
        std::string sinkId;
        sptr<IPrepareDInputCallback> preCallback = nullptr;

        DInputClientRelayPrepareInfo(std::string sourceId, std::string sinkid,
            sptr<IPrepareDInputCallback> prepareCallback)
            : srcId(sourceId), sinkId(sinkid), preCallback(prepareCallback) {}
    };
    struct DInputClientRelayUnprepareInfo {
        std::string srcId;
        std::string sinkId;
        sptr<IUnprepareDInputCallback> unpreCallback = nullptr;
        DInputClientRelayUnprepareInfo(std::string sourceId, std::string sinkid,
            sptr<IUnprepareDInputCallback> unprepareCallback)
            : srcId(sourceId), sinkId(sinkid), unpreCallback(unprepareCallback) {}
    };

    struct DInputClientStartDhidInfo {
        std::string srcId;
        std::string sinkId;
        std::vector<std::string> dhIds;
        sptr<IStartStopDInputsCallback> callback = nullptr;
    };
    struct DInputClientStopDhidInfo {
        std::string srcId;
        std::string sinkId;
        std::vector<std::string> dhIds;
        sptr<IStartStopDInputsCallback> callback = nullptr;
    };

    struct DInputClientStartTypeInfo {
        std::string srcId;
        std::string sinkId;
        uint32_t inputTypes;
        sptr<IStartDInputCallback> callback = nullptr;
        DInputClientStartTypeInfo(std::string sourceId, std::string sinkid, uint32_t types,
            sptr<IStartDInputCallback> cb) : srcId(sourceId), sinkId(sinkid), inputTypes(types), callback(cb) {}
    };
    struct DInputClientStopTypeInfo {
        std::string srcId;
        std::string sinkId;
        uint32_t inputTypes;
        sptr<IStopDInputCallback> callback = nullptr;
        DInputClientStopTypeInfo(std::string sourceId, std::string sinkid, uint32_t types,
            sptr<IStopDInputCallback> cb) : srcId(sourceId), sinkId(sinkid), inputTypes(types), callback(cb) {}
    };

    ServiceSourceRunningState serviceRunningState_ = ServiceSourceRunningState::STATE_NOT_START;
    DInputServerType isStartTrans_ = DInputServerType::NULL_SERVER_TYPE;
    std::shared_ptr<DistributedInputSourceManager::DInputSourceListener> statuslistener_;

    std::vector<DInputClientRegistInfo> regCallbacks_;
    std::vector<DInputClientUnregistInfo> unregCallbacks_;
    std::vector<DInputClientPrepareInfo> preCallbacks_;
    std::vector<DInputClientUnprepareInfo> unpreCallbacks_;
    std::vector<DInputClientStartInfo> staCallbacks_;
    std::vector<DInputClientStopInfo> stpCallbacks_;

    std::vector<DInputClientStartDhidInfo> staStringCallbacks_;
    std::vector<DInputClientStopDhidInfo> stpStringCallbacks_;

    std::vector<DInputClientRelayPrepareInfo> relayPreCallbacks_;
    std::vector<DInputClientRelayUnprepareInfo> relayUnpreCallbacks_;
    std::vector<DInputClientStartDhidInfo> relayStaDhidCallbacks_;
    std::vector<DInputClientStopDhidInfo> relayStpDhidCallbacks_;
    std::vector<DInputClientStartTypeInfo> relayStaTypeCallbacks_;
    std::vector<DInputClientStopTypeInfo> relayStpTypeCallbacks_;

    std::set<sptr<IAddWhiteListInfosCallback>> addWhiteListCallbacks_;
    std::set<sptr<IDelWhiteListInfosCallback>> delWhiteListCallbacks_;
    std::set<sptr<ISimulationEventListener>> simulationEventCallbacks_;

    std::map<std::string, int32_t> DeviceMap_;
    std::map<std::string, uint32_t> InputTypesMap_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<DistributedInputSourceEventHandler> handler_;
    std::shared_ptr<DInputSourceManagerEventHandler> callBackHandler_;
    std::vector<InputDeviceId> inputDevice_;
    bool InitAuto();
    void handleStartServerCallback(const std::string& devId);
    std::mutex mutex_;
    std::mutex operationMutex_;
    sptr<StartDScreenListener> startDScreenListener_ = nullptr;
    sptr<StopDScreenListener> stopDScreenListener_ = nullptr;
    sptr<DeviceOfflineListener> deviceOfflineListener_ = nullptr;

    std::mutex valMutex_;
    std::mutex syncNodeInfoMutex_;
    std::map<std::string, std::set<BeRegNodeInfo>> syncNodeInfoMap_;

    void RunRelayPrepareCallback(const std::string &srcId, const std::string &sinkId, const int32_t &status);
    void RunRelayUnprepareCallback(const std::string &srcId, const std::string &sinkId, const int32_t &status);
    void RunRelayStartDhidCallback(const std::string &srcId, const std::string &sinkId, const int32_t &status,
        const std::string &dhids);
    void RunRelayStopDhidCallback(const std::string &srcId, const std::string &sinkId, const int32_t &status,
        const std::string &dhids);
    void RunRelayStartTypeCallback(const std::string &srcId, const std::string &sinkId, const int32_t &status,
        uint32_t inputTypes);
    void RunRelayStopTypeCallback(const std::string &srcId, const std::string &sinkId, const int32_t &status,
        uint32_t inputTypes);

    int32_t RelayStartRemoteInputByType(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStartDInputCallback> callback);
    int32_t RelayStopRemoteInputByType(const std::string &srcId, const std::string &sinkId, const uint32_t &inputTypes,
        sptr<IStopDInputCallback> callback);
    int32_t RelayPrepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IPrepareDInputCallback> callback);
    int32_t RelayUnprepareRemoteInput(const std::string &srcId, const std::string &sinkId,
        sptr<IUnprepareDInputCallback> callback);
    int32_t RelayStartRemoteInputByDhid(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback);
    int32_t RelayStopRemoteInputByDhid(const std::string &srcId, const std::string &sinkId,
        const std::vector<std::string> &dhIds, sptr<IStartStopDInputsCallback> callback);
    bool IsStringDataSame(const std::vector<std::string> &oldDhIds, std::vector<std::string> newDhIds);
    void StringSplitToVector(const std::string &str, const char split, std::vector<std::string> &vecStr);
    void DeleteNodeInfoAndNotify(const std::string& offlineDevId);
    void SendExistVirNodeInfos(sptr<InputNodeListener> listener);
    std::set<BeRegNodeInfo> GetSyncNodeInfo(const std::string& devId);
    void UpdateSyncNodeInfo(const std::string& devId, const std::string& dhId, const std::string &nodeDesc);
    void DeleteSyncNodeInfo(const std::string& devId);
    void UnregisterDHFwkPublisher();
};
} // namespace DistributedInput
} // namespace DistributedHardware
} // namespace OHOS

#endif // DISTRIBUTED_INPUT_SOURCE_MANAGER_SERVICE_H

#include "src/cli/grpc_cli_server.h"

#include <google/protobuf/util/json_util.h>

#include <iostream>
#include <memory>
#include <string>

#include "src/debug_lib/debug_info_maker.h"
#include "src/debug_lib/debug_info_maker.hpp"

using namespace pos;
#define MAX_NUM_CONCURRENT_CLIENTS 1

CommandProcessor* pc;

namespace pos
{
class CLIDebugMessage
{
public:
    std::string sendReceive;
    std::string message;
    uint32_t errorCode;
};
class CLIDebugInfo : public DebugInfoInstance
{
public:
    CLIDebugMessage info;

    virtual DebugInfoOkay
    IsOkay(void) final
    {
        if (info.errorCode != 0)
        {
            return DebugInfoOkay::FAIL;
        }
        return DebugInfoOkay::PASS;
    }
};

CLIDebugInfo cliDebugInfo;
DebugInfoMaker<CLIDebugInfo>* debugCliInfoMaker;
}
// namespace pos

class PosCliServiceImpl final : public PosCli::Service
{
    grpc::Status
    SystemInfo(ServerContext* context, const SystemInfoRequest* request,
        SystemInfoResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSystemInfoCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    StopSystem(ServerContext* context, const StopSystemRequest* request,
        StopSystemResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteStopSystemCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    GetSystemProperty(ServerContext* context, const GetSystemPropertyRequest* request,
        GetSystemPropertyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteGetSystemPropertyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SetSystemProperty(ServerContext* context, const SetSystemPropertyRequest* request,
        SetSystemPropertyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSetSystemPropertyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    StartTelemetry(ServerContext* context, const StartTelemetryRequest* request,
        StartTelemetryResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteStartTelemetryCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ResetEventWrr(ServerContext* context, const ResetEventWrrRequest* request,
        ResetEventWrrResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteResetEventWrrCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ResetMbr(ServerContext* context, const ResetMbrRequest* request,
        ResetMbrResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteResetMbrCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    StopRebuilding(ServerContext* context, const StopRebuildingRequest* request,
        StopRebuildingResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteStopRebuildingCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    DumpMemorySnapshot(ServerContext* context, const DumpMemorySnapshotRequest* request,
        DumpMemorySnapshotResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteDumpMemorySnapshotCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    StopTelemetry(ServerContext* context, const StopTelemetryRequest* request,
        StopTelemetryResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteStopTelemetryCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SetTelemetryProperty(ServerContext* context, const SetTelemetryPropertyRequest* request,
        SetTelemetryPropertyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSetTelemetryPropertyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    GetTelemetryProperty(ServerContext* context, const GetTelemetryPropertyRequest* request,
        GetTelemetryPropertyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteGetTelemetryPropertyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    UpdateEventWrr(ServerContext* context, const UpdateEventWrrRequest* request,
        UpdateEventWrrResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteUpdateEventWrrCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    AddSpare(ServerContext* context, const AddSpareRequest* request,
        AddSpareResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteAddSpareCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    RemoveSpare(ServerContext* context, const RemoveSpareRequest* request,
        RemoveSpareResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteRemoveSpareCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ReplaceArrayDevice(ServerContext* context, const ReplaceArrayDeviceRequest* request,
        ReplaceArrayDeviceResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteReplaceArrayDeviceCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    CreateArray(ServerContext* context, const CreateArrayRequest* request,
        CreateArrayResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteCreateArrayCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    AutocreateArray(ServerContext* context, const AutocreateArrayRequest* request,
        AutocreateArrayResponse* reply) override
    {
        POS_TRACE_INFO(EID(CLI_MSG_RECEIVED), "message: {}", request->ShortDebugString());

        grpc::Status status = pc->ExecuteAutocreateArrayCommand(request, reply);

        POS_TRACE_INFO(EID(CLI_MSG_SENT), "message: {}", reply->ShortDebugString());

        return status;
    }

    grpc::Status
    DeleteArray(ServerContext* context, const DeleteArrayRequest* request,
        DeleteArrayResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteDeleteArrayCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    MountArray(ServerContext* context, const MountArrayRequest* request,
        MountArrayResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteMountArrayCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    UnmountArray(ServerContext* context, const UnmountArrayRequest* request,
        UnmountArrayResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteUnmountArrayCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ListArray(ServerContext* context, const ListArrayRequest* request,
        ListArrayResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteListArrayCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ArrayInfo(ServerContext* context, const ArrayInfoRequest* request,
        ArrayInfoResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteArrayInfoCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    RebuildArray(ServerContext* context, const RebuildArrayRequest* request,
        RebuildArrayResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteRebuildArrayCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SetLogLevel(ServerContext* context, const SetLogLevelRequest* request,
        SetLogLevelResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSetLogLevelCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SetLogPreference(ServerContext* context, const SetLogPreferenceRequest* request,
        SetLogPreferenceResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSetLogPreferenceCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    LoggerInfo(ServerContext* context, const LoggerInfoRequest* request,
        LoggerInfoResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteLoggerInfoCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    GetLogLevel(ServerContext* context, const GetLogLevelRequest* request,
        GetLogLevelResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteGetLogLevelCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ApplyLogFilter(ServerContext* context, const ApplyLogFilterRequest* request,
        ApplyLogFilterResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteApplyLogFilterCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    CreateDevice(ServerContext* context, const CreateDeviceRequest* request,
        CreateDeviceResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteCreateDeviceCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ScanDevice(ServerContext* context, const ScanDeviceRequest* request,
        ScanDeviceResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteScanDeviceCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ListDevice(ServerContext* context, const ListDeviceRequest* request,
        ListDeviceResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteListDeviceCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    GetSmartLog(ServerContext* context, const GetSmartLogRequest* request,
        GetSmartLogResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteGetSmartLogCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    CreateSubsystem(ServerContext* context, const CreateSubsystemRequest* request,
        CreateSubsystemResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteCreateSubsystemCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    DeleteSubsystem(ServerContext* context, const DeleteSubsystemRequest* request,
        DeleteSubsystemResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteDeleteSubsystemCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    AddListener(ServerContext* context, const AddListenerRequest* request,
        AddListenerResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteAddListenerCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    RemoveListener(ServerContext* context, const RemoveListenerRequest* request,
        RemoveListenerResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteRemoveListenerCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ListListener(ServerContext* context, const ListListenerRequest* request,
        ListListenerResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteListListenerCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SetListenerAnaState(ServerContext* context, const SetListenerAnaStateRequest* request,
        SetListenerAnaStateResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSetListenerAnaStateCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ListSubsystem(ServerContext* context, const ListSubsystemRequest* request,
        ListSubsystemResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteListSubsystemCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SubsystemInfo(ServerContext* context, const SubsystemInfoRequest* request,
        SubsystemInfoResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSubsystemInfoCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    CreateTransport(ServerContext* context, const CreateTransportRequest* request,
        CreateTransportResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteCreateTransportCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    ListTransport(ServerContext* context, const ListTransportRequest* request,
        ListTransportResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteListTransportCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }

        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    CreateVolume(ServerContext* context, const CreateVolumeRequest* request,
        CreateVolumeResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteCreateVolumeCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    MountVolume(ServerContext* context, const MountVolumeRequest* request,
        MountVolumeResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteMountVolumeCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    UnmountVolume(ServerContext* context, const UnmountVolumeRequest* request,
        UnmountVolumeResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteUnmountVolumeCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    DeleteVolume(ServerContext* context, const DeleteVolumeRequest* request,
        DeleteVolumeResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteDeleteVolumeCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    SetVolumeProperty(ServerContext* context, const SetVolumePropertyRequest* request,
        SetVolumePropertyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteSetVolumePropertyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status ListVolume(ServerContext* context, const ListVolumeRequest* request, ListVolumeResponse* reply) override
    {
        _LogCliRequest(request, request->command());
        grpc::Status status = pc->ExecuteListVolumeCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status QosCreateVolumePolicy(ServerContext* context, const QosCreateVolumePolicyRequest* request, QosCreateVolumePolicyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteQosCreateVolumePolicyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status VolumeInfo(ServerContext* context, const VolumeInfoRequest* request, VolumeInfoResponse* reply) override
    {
        _LogCliRequest(request, request->command());
        grpc::Status status = pc->ExecuteVolumeInfoCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }
    grpc::Status VolumeRename(ServerContext* context, const VolumeRenameRequest* request, VolumeRenameResponse* reply) override
    {
        _LogCliRequest(request, request->command());
        grpc::Status status = pc->ExecuteVolumeRenameCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }
    grpc::Status ListQOSPolicy(ServerContext* context, const ListQOSPolicyRequest* request, ListQOSPolicyResponse* reply) override
    {
        _LogCliRequest(request, request->command());
        grpc::Status status = pc->ExecuteListQOSPolicyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

    grpc::Status
    QosResetVolumePolicy(ServerContext* context, const QosResetVolumePolicyRequest* request,
        QosResetVolumePolicyResponse* reply) override
    {
        _LogCliRequest(request, request->command());

        grpc::Status status = pc->ExecuteQosResetVolumePolicyCommand(request, reply);
        if (context->IsCancelled())
        {
            _LogGrpcTimeout(request, reply);
            return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
        }
        _LogCliResponse(reply, status, reply->result().status().code(), request->command());

        return status;
    }

  grpc::Status
  ListWBT(ServerContext* context, const ListWBTRequest* request,
                  ListWBTResponse* reply) override
  {
    _LogCliRequest(request);

    grpc::Status status = pc->ExecuteListWBTCommand(request, reply);
    if (context->IsCancelled()) {
      _LogGrpcTimeout(request, reply);
      return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
    }
    _LogCliResponse(reply, status, reply->result().status().code());

    return status;
  }

  grpc::Status
  WBT(ServerContext* context, const WBTRequest* request,
                  WBTResponse* reply) override
  {
    _LogCliRequest(request);

    grpc::Status status = pc->ExecuteWBTCommand(request, reply);
    if (context->IsCancelled()) {
      _LogGrpcTimeout(request, reply);
      return Status(StatusCode::CANCELLED, GRPC_TIMEOUT_MESSAGE);
    }
    _LogCliResponse(reply, status, reply->result().status().code());

    return status;
  }
};

void
_LogGrpcTimeout(const google::protobuf::Message* request, const google::protobuf::Message* reply)
{
    std::string reqJson = "";
    std::string replyJson = "";
    google::protobuf::util::JsonOptions printOptions;
    printOptions.always_print_primitive_fields = true;
    MessageToJsonString(*request, &reqJson, printOptions);
    MessageToJsonString(*reply, &replyJson, printOptions);

    POS_TRACE_TRACE(EID(CLI_TIMEOUT_OR_CANCELLED), "request: {}, reply: {}",
        reqJson, replyJson);
}

void
_LogCliRequest(const google::protobuf::Message* request, std::string command)
{
    pos::cliDebugInfo.info.sendReceive = "ReceiveCLI";
    pos::cliDebugInfo.info.message = request->ShortDebugString();
    pos::cliDebugInfo.info.errorCode = grpc::StatusCode::OK;
    pos::debugCliInfoMaker->AddDebugInfo();
    logger()->SetCommand(command);
    POS_TRACE_TRACE(EID(CLI_MSG_RECEIVED), "request: {}", request->ShortDebugString());
    logger()->SetCommand("");
}

void
_LogCliResponse(const google::protobuf::Message* reply, const grpc::Status status, uint32_t jsonStatusCode, std::string command)
{

    std::string replyJson = "";
    google::protobuf::util::JsonOptions printOptions;
    printOptions.always_print_primitive_fields = true;
    MessageToJsonString(*reply, &replyJson, printOptions);
    pos::cliDebugInfo.info.sendReceive = "SendCLI";
    pos::cliDebugInfo.info.message = reply->ShortDebugString();
    pos::cliDebugInfo.info.errorCode = jsonStatusCode;
    pos::debugCliInfoMaker->AddDebugInfo();

    logger()->SetCommand(command);
    POS_TRACE_TRACE(EID(CLI_MSG_SENT), "response: {}, gRPC_error_code: {}, gRPC_error_details: {}, gRPC_error_essage: {}",
        replyJson, status.error_code(), status.error_details(), status.error_message());
    logger()->SetCommand("");
}

void
RunGrpcServer()
{
    pc = new CommandProcessor();

    std::string server_address(GRPC_CLI_SERVER_SOCKET_ADDRESS);
    PosCliServiceImpl service;
    pos::debugCliInfoMaker = new pos::DebugInfoMaker<CLIDebugInfo>(&cliDebugInfo, "Cli", 500);
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    grpc::ResourceQuota rq;
    rq.SetMaxThreads(MAX_NUM_CONCURRENT_CLIENTS + 1);
    builder.SetResourceQuota(rq);
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Grpc CLI server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
    delete pos::debugCliInfoMaker;
}

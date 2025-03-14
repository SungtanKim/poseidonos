/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/cli/set_listener_ana_state_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/network/nvmf_target.h"
#include "src/logger/logger.h"

namespace pos_cli
{
SetListenerAnaStateCommand::SetListenerAnaStateCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
SetListenerAnaStateCommand::~SetListenerAnaStateCommand(void)
{
}
// LCOV_EXCL_STOP

string
SetListenerAnaStateCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;

    int ret = 0;
    ret = _SetListenerAnaState(doc);
    if (ret != SUCCESS)
    {
        return jFormat.MakeResponse(
            "SETLISTENERANASTATE", rid, ret, errorMessage, GetPosInfo());
    }

    int event = EID(CLI_SET_LISTENER_ANA_STATE_SUCCESS);
    POS_TRACE_INFO(event, "");
    return jFormat.MakeResponse(
        "SETLISTENERANASTATE", rid, SUCCESS,
        "Address ( " + doc["param"]["target_address"].get<string>() + " ) set to Subsystem ( " + subnqn + " )'s ana state: " + doc["param"]["ana_state"].get<string>(), GetPosInfo());
}

int
SetListenerAnaStateCommand::_SetListenerAnaState(json& doc)
{
    SpdkRpcClient rpcClient;
    NvmfTarget target;
    subnqn = doc["param"]["name"].get<string>();

    if (nullptr == target.FindSubsystem(subnqn))
    {
        errorMessage = "Failed to set listener's ana state. Requested Subsystem does not exist or invalid subnqn. ";
        return FAIL;
    }

    auto ret = rpcClient.SubsystemSetListenerAnaState(
        subnqn,
        doc["param"]["transport_type"].get<string>(),
        DEFAULT_ADRFAM,
        doc["param"]["target_address"].get<string>(),
        doc["param"]["transport_service_id"].get<string>(),
        doc["param"]["ana_state"].get<string>());
    if (ret.first != SUCCESS)
    {
        errorMessage = "Failed to set listener's ana state. " + ret.second;
    }
    return ret.first;
}
} // namespace pos_cli

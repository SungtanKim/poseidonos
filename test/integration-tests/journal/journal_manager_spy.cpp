#include "test/integration-tests/journal/journal_manager_spy.h"

#include <thread>
#include <typeinfo>

#include "src/include/smart_ptr_type.h"
#include "src/journal_manager/checkpoint/checkpoint_handler.h"
#include "src/journal_manager/checkpoint/dirty_map_manager.h"
#include "src/journal_manager/journal_writer.h"
#include "src/journal_manager/log/log_buffer_parser.h"
#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"
#include "src/journal_manager/log_buffer/reset_log_group.h"
#include "src/journal_manager/log_write/buffer_offset_allocator.h"
#include "src/journal_manager/log_write/journal_volume_event_handler.h"
#include "src/journal_manager/replay/replay_handler.h"
#include "src/journal_manager/status/journal_status_provider.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/rocksdb_log_buffer/rocksdb_log_buffer.h"
#include "test/integration-tests/journal/journal_configuration_spy.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;

namespace pos
{
JournalManagerSpy::JournalManagerSpy(TelemetryPublisher* tp, IArrayInfo* array, IStateControl* stateSub, std::string logFileName)
: JournalManager(tp, array, stateSub)
{
    delete logGroupReleaser;
    logGroupReleaser = new LogGroupReleaserSpy();

    delete logBuffer;
    uint32_t arrayId = 0;
    logBuffer = new JournalLogBuffer(new MockFileIntf(logFileName, arrayId, MetaFileType::General, MetaVolumeType::NvRamVolume));

    LogFileName = logFileName;

    // TODO (cheolho.kang): Move codes about the MockEventScheduler to the JournalManagerTestFixture
    eventScheduler = new NiceMock<MockEventScheduler>;
    ON_CALL(*eventScheduler, EnqueueEvent).WillByDefault([&](EventSmartPtr event) {
        std::string targetEventName(typeid(*event.get()).name());
        auto searchResult = eventReplaceTable.find(targetEventName);
        if (searchResult != eventReplaceTable.end())
        {
            EventSmartPtr faultEvent = searchResult->second;
            std::thread eventExecution(&Event::Execute, faultEvent);
            eventExecution.detach();
            eventReplaceTable.erase(searchResult);
        }
        else
        {
            std::thread eventExecution(&Event::Execute, event);
            eventExecution.detach();
        }
    });
}

JournalManagerSpy::~JournalManagerSpy(void)
{
    logBuffer->Dispose();

    delete eventScheduler;
}

int
JournalManagerSpy::InitializeForTest(TelemetryClient* telemetryClient, Mapper* mapper, Allocator* allocator, IVolumeInfoManager* volumeManager)
{
    int ret = JournalManager::_InitConfigAndPrepareLogBuffer(nullptr);
    if (ret < 0)
    {
        return ret;
    }

    _InitModules(telemetryClient, mapper->GetIVSAMap(), mapper->GetIStripeMap(),
        mapper->GetIMapFlush(),
        allocator->GetISegmentCtx(),
        allocator->GetIWBStripeAllocator(),
        allocator->GetIContextManager(), allocator->GetIContextReplayer(),
        volumeManager, eventScheduler);

    if (journalingStatus.Get() != WAITING_TO_BE_REPLAYED)
    {
        ret = JournalManager::_Reset();
    }

    LogGroupReleaserSpy* releaserTester =
        dynamic_cast<LogGroupReleaserSpy*>(logGroupReleaser);
    if (releaserTester == nullptr)
    {
        ret = -1;
    }

    return ret;
}

int
JournalManagerSpy::DoRecoveryForTest(void)
{
    return JournalManager::_DoRecovery();
}

void
JournalManagerSpy::DeleteLogBuffer(void)
{
    logBuffer->Delete();
}

void
JournalManagerSpy::ResetJournalConfiguration(JournalConfiguration* journalConfig)
{
    delete config;
    config = journalConfig;

    if (config->IsRocksdbEnabled())
    {
        delete logBuffer;
        logBuffer = new RocksDBLogBuffer(LogFileName);
    }
}

void
JournalManagerSpy::StartCheckpoint(void)
{
    ((LogGroupReleaserSpy*)(logGroupReleaser))->TriggerCheckpoint();
}

void
JournalManagerSpy::SetTriggerCheckpoint(bool val)
{
    ((LogGroupReleaserSpy*)(logGroupReleaser))->triggerCheckpoint = val;
}

bool
JournalManagerSpy::IsCheckpointEnabled(void)
{
    return ((LogGroupReleaserSpy*)logGroupReleaser)->triggerCheckpoint;
}

uint64_t
JournalManagerSpy::GetLogBufferSize(void)
{
    return config->GetLogBufferSize();
}

uint64_t
JournalManagerSpy::GetLogGroupSize(void)
{
    return config->GetLogGroupSize();
}

int
JournalManagerSpy::GetNumLogGroups(void)
{
    return config->GetNumLogGroups();
}

uint64_t
JournalManagerSpy::GetMetaPageSize(void)
{
    return config->GetMetaPageSize();
}

bool
JournalManagerSpy::IsCheckpointCompleted(void)
{
    return (logGroupReleaser->GetFullLogGroups().size() == 0);
}

int
JournalManagerSpy::GetNumDirtyMap(int logGroupId)
{
    return dirtyMapManager->GetDirtyList(logGroupId).size();
}

int
JournalManagerSpy::GetLogs(LogList& logList)
{
    if (config->IsEnabled() == false)
    {
        config->SetLogBufferSize(0, nullptr);

        bool fileExist = logBuffer->DoesLogFileExist();
        if (fileExist == false)
        {
            return 0;
        }
        else
        {
            return _GetLogsFromBuffer(logList);
        }
    }
    return _GetLogsFromBuffer(logList);
}

int
JournalManagerSpy::_GetLogsFromBuffer(LogList& logList)
{
    LogBufferParser parser;

    int result = 0;
    uint64_t groupSize = config->GetLogGroupSize();
    for (int groupId = 0; groupId < config->GetNumLogGroups(); groupId++)
    {
        void* logGroupBuffer = calloc(groupSize, sizeof(char));
        result = logBuffer->ReadLogBuffer(groupId, logGroupBuffer);
        if (result != 0)
        {
            free(logGroupBuffer);
            break;
        }

        result = parser.GetLogs(logGroupBuffer, groupId, groupSize, logList);
        if (result != 0)
        {
            free(logGroupBuffer);
            break;
        }
        free(logGroupBuffer);
    }
    return result;
}

uint64_t
JournalManagerSpy::GetNumLogsAdded(void)
{
    return bufferAllocator->GetNumLogsAdded();
}

int
JournalManagerSpy::VolumeDeleted(int volId)
{
    int ret = 0;
    ret = volumeEventHandler->WriteVolumeDeletedLog(volId);
    if (ret == 0)
    {
        volumeEventHandler->TriggerMetadataFlush();
    }

    return ret;
}

uint64_t
JournalManagerSpy::GetNextOffset(void)
{
    return bufferAllocator->GetNextOffset();
}

IJournalWriter*
JournalManagerSpy::GetJournalWriter(void)
{
    return journalWriter;
}

IJournalStatusProvider*
JournalManagerSpy::GetStatusProvider(void)
{
    return statusProvider;
}

LogGroupReleaser*
JournalManagerSpy::GetLogGroupReleaser(void)
{
    return logGroupReleaser;
}

void
JournalManagerSpy::ResetVersionedSegmentContext(void)
{
    delete versionedSegCtx;
    versionedSegCtx = _CreateVersionedSegmentCtx();
}

IVersionedSegmentContext*
JournalManagerSpy::GetVersionedSegmentContext(void)
{
    return versionedSegCtx;
}

void
JournalManagerSpy::InjectFaultEvent(const std::type_info& targetEventInfo, EventSmartPtr errorEvent)
{
    std::string targetEventName(targetEventInfo.name());
    eventReplaceTable.insert({targetEventName, errorEvent});
}
} // namespace pos

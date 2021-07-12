#include "src/telemetry/telemetry_client/telemetry_publisher.h"

#include <gtest/gtest.h>
namespace pos
{
TEST(TelemetryPublisher, PublishData_TestUpdateAndCollectItem)
{
    // given
    TelemetryPublisher tp;
    // given 1.
    tp.StopPublishing();
    // when 1.
    int ret = tp.PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 100);
    // then 1.
    EXPECT_EQ(-1, ret);
    // given 2.
    tp.StartPublishing();
    // when 2.
    ret = tp.PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 200);
    // then 2.
    EXPECT_EQ(0, ret);
    // given 3.
    tp.PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 10);
    tp.PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 9);
    TelemetryGeneralMetric log;
    // when 3.
    ret = tp.CollectData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, log);
    // then 3.
    EXPECT_EQ(9, log.GetValue());
    // when 4.
    ret = tp.CollectData("ccc", log);
    // then 4.
    EXPECT_EQ(-1, ret);
}

TEST(TelemetryPublisher, PublishData_TestExceedEntryLimit)
{
    // given
    TelemetryPublisher tp;
    tp.SetMaxEntryLimit(2);
    tp.StartPublishing();
    int ret = tp.PublishData(TEL_ALLOCATOR_ALLOCATORCTX_PENDING_IO_COUNT, 100);
    ret = tp.PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 200);
    // when 1.
    ret = tp.PublishData(TEL_ALLOCATOR_GCMODE, 3);
    ret = tp.GetNumEntries();
    // then 1.
    EXPECT_EQ(2, ret);
    // when 2.
    ret = tp.PublishData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, 300);
    // then 2.
    EXPECT_EQ(0, ret);
    TelemetryGeneralMetric log;
    tp.CollectData(TEL_ALLOCATOR_FREE_SEGMENT_COUNT, log);
    EXPECT_EQ(300, log.GetValue());
}

} // namespace pos

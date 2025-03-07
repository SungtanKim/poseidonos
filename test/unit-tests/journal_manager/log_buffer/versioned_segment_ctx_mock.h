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

#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/versioned_segment_ctx.h"

namespace pos
{
class MockDummyVersionedSegmentCtx : public DummyVersionedSegmentCtx
{
public:
    using DummyVersionedSegmentCtx::DummyVersionedSegmentCtx;
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration, SegmentInfo* loadedSegmentInfos, uint32_t numSegments), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, DecreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, IncreaseOccupiedStripeCount, (int logGroupId, SegmentId segId), (override));
    MOCK_METHOD(SegmentInfoData*, GetUpdatedInfoDataToFlush, (int logGroupId), (override));
    MOCK_METHOD(int, GetNumSegments, (), (override));
    MOCK_METHOD(int, GetNumLogGroups, (), (override));
    MOCK_METHOD(void, ResetFlushedInfo, (int logGroupId), (override));
    MOCK_METHOD(void, ResetInfosAfterSegmentFreed, (SegmentId targetSegmentId), (override));
};

class MockVersionedSegmentCtx : public VersionedSegmentCtx
{
public:
    using VersionedSegmentCtx::VersionedSegmentCtx;
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration, SegmentInfo* loadedSegmentInfos, uint32_t numSegments), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, DecreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, IncreaseOccupiedStripeCount, (int logGroupId, SegmentId segId), (override));
    MOCK_METHOD(SegmentInfoData*, GetUpdatedInfoDataToFlush, (int logGroupId), (override));
    MOCK_METHOD(void, ResetFlushedInfo, (int logGroupId), (override));
    MOCK_METHOD(int, GetNumSegments, (), (override));
    MOCK_METHOD(int, GetNumLogGroups, (), (override));
    MOCK_METHOD(void, Init, (JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfo, uint32_t numSegments,
        std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo), (override));
    MOCK_METHOD(void, ResetInfosAfterSegmentFreed, (SegmentId targetSegmentId), (override));
};

} // namespace pos

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

#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <mutex>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/qos/qos_manager.h"
namespace pos
{
SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfoData* segmentInfoData_,
    RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_, GcCtx* gcCtx_)
: SegmentCtx(tp_, header, segmentInfoData_, nullptr, rebuildCtx_, addrInfo_, gcCtx_)
{
}

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, SegmentCtxHeader* header, SegmentInfoData* segmentInfoData_,
    SegmentList* rebuildSegmentList, RebuildCtx* rebuildCtx_, AllocatorAddressInfo* addrInfo_, GcCtx* gcCtx_)
: totalDataSize(0),
  ctxDirtyVersion(0),
  ctxStoredVersion(0),
  segmentInfos(nullptr),
  rebuildList(rebuildSegmentList),
  rebuildingSegment(UNMAP_SEGMENT),
  initialized(false),
  addrInfo(addrInfo_),
  rebuildCtx(rebuildCtx_),
  gcCtx(gcCtx_),
  tp(tp_)
{
    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
    {
        segmentList[state] = nullptr;
    }

    if (segmentInfoData_ != nullptr)
    {
        uint32_t numSegments = addrInfo->GetnumUserAreaSegments();

        segmentInfoData.data = segmentInfoData_;
        segmentInfos = new SegmentInfo[numSegments];
        for (uint32_t segId = 0; segId < numSegments; segId++)
        {
            // This time, SegmentInfoData is created outside and injected to here, hence we skip initializing it,
            // but just allocate only.
            segmentInfos[segId].AllocateSegmentInfoData(&segmentInfoData.data[segId]);
        }
    }
    else
    {
        segmentInfoData.data = nullptr;
    }

    if (header != nullptr)
    {
        // for UT
        ctxHeader.data = *header;
    }
    else
    {
        ctxHeader.data.sig = SIG_SEGMENT_CTX;
        ctxHeader.data.ctxVersion = 0;
        ctxHeader.data.numValidSegment = 0;
    }
}

SegmentCtx::SegmentCtx(TelemetryPublisher* tp_, RebuildCtx* rebuildCtx_,
    AllocatorAddressInfo* info, GcCtx* gcCtx_, SegmentInfoData* segmentInfoData_)
: SegmentCtx(tp_, nullptr, segmentInfoData_, rebuildCtx_, info, gcCtx_)
{
}

SegmentCtx::~SegmentCtx(void)
{
    Dispose();
}

// Only for UT
void
SegmentCtx::SetSegmentList(SegmentState state, SegmentList* list)
{
    segmentList[state] = list;
}

void
SegmentCtx::SetRebuildList(SegmentList* list)
{
    rebuildList = list;
}

void
SegmentCtx::Init(void)
{
    if (initialized == true)
    {
        return;
    }

    ctxHeader.data.ctxVersion = 0;
    ctxStoredVersion = 0;
    ctxDirtyVersion = 0;

    if (segmentInfoData.data == nullptr)
    {
        uint32_t numSegments = addrInfo->GetnumUserAreaSegments();
        segmentInfos = new SegmentInfo[numSegments];
        segmentInfoData.data = new SegmentInfoData[numSegments];
        for (uint32_t i = 0; i < numSegments ; ++i)
        {
            // This time, SegmentInfoData needs to be newly created "and initialized", hence we use AllocateAndInitSegmentInfoData()
            segmentInfos[i].AllocateAndInitSegmentInfoData(&segmentInfoData.data[i]);
            segmentInfos[i].SetArrayId(addrInfo->GetArrayId());
            segmentInfos[i].SetSegmentId((SegmentId)i);
        }
    }

    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
    {
        if (segmentList[state] == nullptr)
        {
            segmentList[state] = new SegmentList(addrInfo->GetArrayId(), (SegmentState)state);
        }
    }

    if (rebuildList == nullptr)
    {
        rebuildList = new SegmentList(addrInfo->GetArrayId(), SegmentState::START /* don't care */);
    }

    _UpdateSectionInfo();
    _RebuildSegmentList();

    initialized = true;
}

void
SegmentCtx::_UpdateSectionInfo(void)
{
    uint64_t currentOffset = 0;

    // SC_HEADER
    ctxHeader.InitAddressInfoWithItsData(currentOffset);
    currentOffset += ctxHeader.GetSectionSize();

    // SC_SEGMENT_INFO
    uint64_t segmentInfoSize = sizeof(SegmentInfoData) * addrInfo->GetnumUserAreaSegments();
    segmentInfoData.InitAddressInfo(
        (char*)segmentInfoData.data, currentOffset, segmentInfoSize);
    currentOffset += segmentInfoData.GetSectionSize();

    totalDataSize = currentOffset;
}

void
SegmentCtx::Dispose(void)
{
    if (initialized == false)
    {
        return;
    }

    if (segmentInfos != nullptr)
    {
        delete[] segmentInfos;
        segmentInfos = nullptr;
    }

    if (segmentInfoData.data != nullptr)
    {
        delete[] segmentInfoData.data;
        segmentInfoData.data = nullptr;
    }

    for (int state = SegmentState::FREE; state < SegmentState::NUM_STATES; state++)
    {
        if (segmentList[state] != nullptr)
        {
            delete segmentList[state];
            segmentList[state] = nullptr;
        }
    }

    if (rebuildList != nullptr)
    {
        delete rebuildList;
        rebuildList = nullptr;
    }

    initialized = false;
}

bool
SegmentCtx::MoveToFreeState(SegmentId segId)
{
    bool segmentFreed = segmentInfos[segId].MoveVictimToFree();
    if (segmentFreed == true)
    {
        bool removed = segmentList[SegmentState::VICTIM]->RemoveFromList(segId);
        POS_TRACE_INFO(EID(ALLOCATOR_COPIER_FREE_SEGMENT_SUCCESS),
            "segment_id:{}, is_removed_from_victim_segment_list:{}", segId, removed);
        _SegmentFreed(segId);
    }
    else
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_SEGMENT_REMOVAL_FAILURE_ALREADY_FREE_SEGMENT),
            "segment_id:{}, state:{}", segId, segmentInfos[segId].GetState());
    }
    return segmentFreed;
}

void
SegmentCtx::ValidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    _IncreaseValidBlockCount(segId, blks.numBlks);
}

void
SegmentCtx::_IncreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    uint32_t increasedValue = segmentInfos[segId].IncreaseValidBlockCount(cnt);
    if (increasedValue > addrInfo->GetblksPerSegment())
    {
        POS_TRACE_CRITICAL(EID(ALLOCATOR_VALID_BLOCK_COUNT_OVERFLOW),
            "segment_id:{} increase_count:{} total_valid_block_count:{}", segId, cnt, increasedValue);
        assert(false);
    }
}

bool
SegmentCtx::InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    return _DecreaseValidBlockCount(segId, blks.numBlks, allowVictimSegRelease);
}

bool
SegmentCtx::_DecreaseValidBlockCount(SegmentId segId, uint32_t cnt, bool allowVictimSegRelease)
{
    auto result = segmentInfos[segId].DecreaseValidBlockCount(cnt, allowVictimSegRelease);

    if (result.second == SegmentState::ERROR)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED),
            "segId{} cnt:{} , allow {}", segId, cnt, allowVictimSegRelease);
        assert(false);
    }

    bool segmentFreed = result.first;
    if (segmentFreed == true)
    {
        SegmentState prevState = result.second;
        bool removed = segmentList[prevState]->RemoveFromList(segId);

        POS_TRACE_DEBUG(EID(ALLOCATOR_TARGET_SEGMENT_FREE_DONE),
            "segment_id:{}, prev_state:{}, is_removed_from_victim_segment_list:{}, array_id:{}",
            segId, prevState, removed, addrInfo->GetArrayId());
        _SegmentFreed(segId);
    }

    return segmentFreed;
}

uint32_t
SegmentCtx::GetValidBlockCount(SegmentId segId)
{
    return segmentInfos[segId].GetValidBlockCount();
}

bool
SegmentCtx::UpdateOccupiedStripeCount(StripeId lsid)
{
    SegmentId segId = lsid / addrInfo->GetstripesPerSegment();
    return _IncreaseOccupiedStripeCount(segId);
}

bool
SegmentCtx::_IncreaseOccupiedStripeCount(SegmentId segId)
{
    uint32_t occupiedStripeCount = segmentInfos[segId].IncreaseOccupiedStripeCount();
    bool segmentFreed = false;

    if (occupiedStripeCount == addrInfo->GetstripesPerSegment())
    {
        // Only 1 thread reaches here
        SegmentState prevState = segmentInfos[segId].GetState();
        segmentList[prevState]->RemoveFromList(segId);

        segmentFreed = segmentInfos[segId].MoveToSsdStateOrFreeStateIfItBecomesEmpty();
        if (segmentFreed == true)
        {
            _SegmentFreed(segId);
        }
        else
        {
            segmentList[SegmentState::SSD]->AddToList(segId);
        }
    }

    return segmentFreed;
}

uint32_t
SegmentCtx::GetOccupiedStripeCount(SegmentId segId)
{
    return segmentInfos[segId].GetOccupiedStripeCount();
}

void
SegmentCtx::AfterLoad(char* buf)
{
    SegmentCtxHeader* header = reinterpret_cast<SegmentCtxHeader*>(buf);
    assert(header->sig == SIG_SEGMENT_CTX);

    // SC_HEADER
    ctxHeader.CopyFrom(buf);
    // SC_SEGMENT_INFO
    segmentInfoData.CopyFrom(buf);

    POS_TRACE_DEBUG(EID(ALLOCATOR_FILE_LOAD_ERROR),
        "SegmentCtx file loaded:{}", ctxHeader.data.ctxVersion);
    ctxStoredVersion = ctxHeader.data.ctxVersion;
    ctxDirtyVersion = ctxHeader.data.ctxVersion + 1;
    _RebuildSegmentList();
}

void
SegmentCtx::_RebuildSegmentList(void)
{
    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; ++state)
    {
        segmentList[state]->Reset();
    }

    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        SegmentState state = segmentInfos[segId].GetState();
        segmentList[state]->AddToList(segId);

        if (state != SegmentState::FREE)
        {
            POS_TRACE_DEBUG(EID(ALLOCATOR_SEGMENT_ADDED_TO_LIST),
                "array_id: {}, segment_id:{}, state:{}, valid_block_count: {}, occupied_stripe_count: {}",
                addrInfo->GetArrayId(), segId, state, segmentInfos[segId].GetValidBlockCount(), segmentInfos[segId].GetOccupiedStripeCount());
        }
    }
}

void
SegmentCtx::BeforeFlush(char* buf)
{
    ctxHeader.data.ctxVersion = ctxDirtyVersion++;

    std::lock_guard<std::mutex> lock(segCtxLock);
    // SC_HEADER
    ctxHeader.CopyTo(buf);
    // SC_SEGMENT_INFO
    segmentInfoData.CopyTo(buf);
}

void
SegmentCtx::AfterFlush(char* buf)
{
    SegmentCtxHeader* header = reinterpret_cast<SegmentCtxHeader*>(buf);
    assert(header->sig == SIG_SEGMENT_CTX);

    ctxStoredVersion = header->ctxVersion;

    POS_TRACE_DEBUG(EID(ALLOCATOR_DEBUG),
        "SegmentCtx stored, array_id: {}, context_version: {}",
        addrInfo->GetArrayId(), ctxStoredVersion);
}

ContextSectionAddr
SegmentCtx::GetSectionInfo(int section)
{
    if (section == SC_HEADER)
    {
        return ctxHeader.GetSectionInfo();
    }
    else if (section == SC_SEGMENT_INFO)
    {
        return segmentInfoData.GetSectionInfo();
    }
    else
    {
        assert(false);
    }
}

uint64_t
SegmentCtx::GetStoredVersion(void)
{
    return ctxStoredVersion;
}

void
SegmentCtx::ResetDirtyVersion(void)
{
    ctxDirtyVersion = 0;
}

int
SegmentCtx::GetNumSections(void)
{
    return NUM_SEGMENT_CTX_SECTION;
}

uint64_t
SegmentCtx::GetTotalDataSize(void)
{
    return ctxHeader.GetSectionSize() + segmentInfoData.GetSectionSize();
}

SegmentState
SegmentCtx::GetSegmentState(SegmentId segId)
{
    return segmentInfos[segId].GetState();
}

void
SegmentCtx::AllocateSegment(SegmentId segId)
{
    // Only used by context replayer. Free segment list will be rebuilt after replay is completed
    segmentInfos[segId].MoveToNvramState();
}

SegmentId
SegmentCtx::AllocateFreeSegment(void)
{
    while (true)
    {
        SegmentId segId = segmentList[SegmentState::FREE]->PopSegment();
        if (segId == UNMAP_SEGMENT)
        {
            POS_TRACE_DEBUG(EID(ALLOCATOR_ALLOCATE_FAILURE_NO_FREE_SEGMENT),
                "segment_id:{}, free_segment_count:{}, array_id:{}",
                segId, GetNumOfFreeSegmentWoLock(), addrInfo->GetArrayId());
            break;
        }
        else
        {
            segmentInfos[segId].MoveToNvramState();
            segmentList[SegmentState::NVRAM]->AddToList(segId);

            int numFreeSegment = _OnNumFreeSegmentChanged();
            POS_TRACE_DEBUG(EID(ALLOCATOR_FREE_SEGMENT_ALLOCATION_SUCCESS),
                "segment_id:{}, free_segment_count:{}, array_id:{}",
                segId, numFreeSegment, addrInfo->GetArrayId());

            return segId;
        }
    }

    return UNMAP_SEGMENT;
}

uint64_t
SegmentCtx::GetNumOfFreeSegment(void)
{
    return segmentList[SegmentState::FREE]->GetNumSegments();
}

uint64_t
SegmentCtx::GetNumOfFreeSegmentWoLock(void)
{
    return segmentList[SegmentState::FREE]->GetNumSegmentsWoLock();
}

int
SegmentCtx::GetAllocatedSegmentCount(void)
{
    return addrInfo->GetnumUserAreaSegments() - segmentList[SegmentState::FREE]->GetNumSegments();
}

SegmentId
SegmentCtx::AllocateGCVictimSegment(void)
{
    SegmentId victimSegmentId = UNMAP_SEGMENT;
    while ((victimSegmentId = _FindMostInvalidSSDSegment()) != UNMAP_SEGMENT)
    {
        bool successToSetVictim = _SetVictimSegment(victimSegmentId);
        if (successToSetVictim == true) break;
    }

    return victimSegmentId;
}

SegmentId
SegmentCtx::_FindMostInvalidSSDSegment(void)
{
    uint32_t numUserAreaSegments = addrInfo->GetnumUserAreaSegments();
    SegmentId victimSegment = UNMAP_SEGMENT;
    uint32_t minValidCount = addrInfo->GetblksPerSegment();
    for (SegmentId segId = 0; segId < numUserAreaSegments; ++segId)
    {
        uint32_t cnt = segmentInfos[segId].GetValidBlockCountIfSsdState();
        if (cnt < minValidCount)
        {
            victimSegment = segId;
            minValidCount = cnt;
        }
    }

    POS_TRACE_DEBUG(EID(ALLOCATE_GC_VICTIM),
        "victim_segment:{}, min_valid_count:{}",
        victimSegment, minValidCount);

    return victimSegment;
}

bool
SegmentCtx::_SetVictimSegment(SegmentId victimSegment)
{
    assert(victimSegment != UNMAP_SEGMENT);

    bool stateChanged = segmentInfos[victimSegment].MoveToVictimState();
    POS_TRACE_DEBUG(EID(ALLOCATE_GC_VICTIM),
                "victim_segment:{}, state_changed:{}",
                victimSegment, stateChanged);
    if (stateChanged == true)
    {
        // This segment is in SSD LIST or REBUILD LIST
        bool isContained = rebuildList->Contains(victimSegment);
        if (true == isContained)
        {
            // do nothing. this segment will be return to the victim list when rebuild is completed
            POS_TRACE_DEBUG(EID(ALLOCATE_GC_VICTIM),
                "rebuild_list_contains:{}",
                isContained);
        }
        else
        {
            segmentList[SegmentState::SSD]->RemoveFromList(victimSegment);
            segmentList[SegmentState::VICTIM]->AddToList(victimSegment);
        }

        _UpdateTelemetryOnVictimSegmentAllocation(victimSegment);

        POS_TRACE_DEBUG(EID(ALLOCATE_GC_VICTIM),
            "victim_segment_id:{}, free_segment_count:{}, array_id:{}",
            victimSegment, GetNumOfFreeSegmentWoLock(), addrInfo->GetArrayId());
    }

    return stateChanged;
}

void
SegmentCtx::_UpdateTelemetryOnVictimSegmentAllocation(SegmentId victimSegment)
{
    POSMetricValue validCount;
    validCount.gauge = segmentInfos[victimSegment].GetValidBlockCount();
    tp->PublishData(TEL30010_ALCT_VICTIM_SEG_INVALID_PAGE_CNT, validCount, MT_GAUGE);

    POSMetricValue victimSegmentId;
    victimSegmentId.gauge = victimSegment;
    tp->PublishData(TEL30002_ALCT_GCVICTIM_SEG, victimSegmentId, MT_GAUGE);
}

void
SegmentCtx::_SegmentFreed(SegmentId segmentId)
{
    if (rebuildingSegment != segmentId)
    {
        if (true == rebuildList->RemoveFromList(segmentId))
        {
            _FlushRebuildSegmentList();
            POS_TRACE_DEBUG(EID(ALLOCATOR_TARGET_SEGMENT_FREE_REMOVAL_FROM_REBUILD_LIST_DONE),
            "segmentId:{} in Rebuild Target has been Freed by GC", segmentId);
        }
    }
    else
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_TARGET_SEGMENT_UNDER_REBUILDING),
            "segment_id:{}", segmentId);
        return;
    }

    segmentList[SegmentState::FREE]->AddToList(segmentId);

    int numOfFreeSegments = _OnNumFreeSegmentChanged();

    POS_TRACE_DEBUG(EID(ALLOCATOR_TARGET_SEGMENT_FREE_DONE),
        "segment_id:{}, free_segment_count:{}, array_id:{}",
        segmentId, numOfFreeSegments, addrInfo->GetArrayId());
}

int
SegmentCtx::_OnNumFreeSegmentChanged(void)
{
    int numOfFreeSegments = GetNumOfFreeSegment();

    QosManagerSingleton::Instance()->SetGcFreeSegment(numOfFreeSegments, addrInfo->GetArrayId());

    gcCtx->UpdateCurrentGcMode(numOfFreeSegments);

    POSMetricValue v;
    v.gauge = numOfFreeSegments;
    tp->PublishData(TEL30000_ALCT_FREE_SEG_CNT, v, MT_GAUGE);

    return numOfFreeSegments;
}

void
SegmentCtx::ResetSegmentsStates(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_RESET_SEGMENTS), "SegmentCtx::ResetSegmentsStates");

    int validCount = 0;
    int occupiedStripeCount = 0;

    for (uint32_t segId = 0; segId < addrInfo->GetnumUserAreaSegments(); ++segId)
    {
        validCount = GetValidBlockCount(segId);
        occupiedStripeCount = GetOccupiedStripeCount(segId);

        if ((0 == validCount) && (0 == occupiedStripeCount))
        {
            segmentInfos[segId].SetState(SegmentState::FREE);
        }
        else if ((0 <= validCount) && (occupiedStripeCount == (int)addrInfo->GetstripesPerSegment()))
        {
            segmentInfos[segId].SetState(SegmentState::SSD);
        }
        else if ((0 <= validCount) && (0 <= occupiedStripeCount))
        {
            segmentInfos[segId].SetState(SegmentState::NVRAM);
        }
        else
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_FILE_ERROR), "segment id {}, validCount {}, occupiedStripeCount {}",
                            segId, validCount, occupiedStripeCount);
            assert(false);
        }
    }

    _RebuildSegmentList();
    _OnNumFreeSegmentChanged();
}

SegmentId
SegmentCtx::GetRebuildTargetSegment(void)
{
    SegmentId segmentId = UNMAP_SEGMENT;

    while (true)
    {
        segmentId = rebuildList->PopSegment();
        if (segmentId == UNMAP_SEGMENT)
        {
            segmentId = UINT32_MAX;
            break;
        }
        else if (segmentInfos[segmentId].GetState() == SegmentState::FREE)
        {
            segmentList[SegmentState::FREE]->AddToList(segmentId);

            POS_TRACE_DEBUG(EID(ALLOCATOR_SEGMENT_REMOVAL_FAILURE_ALREADY_FREE_SEGMENT),
                "segment_id:{}", segmentId);
            _FlushRebuildSegmentList();
        }
        else
        {
            rebuildingSegment = segmentId;

            POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET_START), "segment_id:{}", segmentId);
            break;
        }
    }

    return segmentId;
}

int
SegmentCtx::_FlushRebuildSegmentList(void)
{
    auto list = rebuildList->GetList();
    if (rebuildingSegment != UNMAP_SEGMENT)
    {
        list.insert(rebuildingSegment);
    }

    return rebuildCtx->FlushRebuildSegmentList(list);
}

int
SegmentCtx::SetRebuildCompleted(SegmentId segId)
{
    if (rebuildingSegment == segId)
    {
        SegmentState state = segmentInfos[segId].GetState();
        segmentList[state]->AddToList(segId);

        _ResetSegmentIdInRebuilding();

        POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_SEGMENT_COMPLETED),
            "segment_id:{}", segId);
        return _FlushRebuildSegmentList();
    }
    else
    {
        POS_TRACE_WARN(EID(UNKNOWN_ALLOCATOR_ERROR),
            "completed_segment:{}, rebuilding_segment:{}", segId, rebuildingSegment);
        return 0;
    }
}

int
SegmentCtx::MakeRebuildTarget(void)
{
    _BuildRebuildSegmentList();

    POS_TRACE_DEBUG(EID(ALLOCATOR_MAKE_REBUILD_TARGET_DONE),
        "num_of_segments_in_rebuild_list:{}", rebuildList->GetNumSegments());

    int flushResult = _FlushRebuildSegmentList();
    if (rebuildList->GetNumSegments() == 0)
    {
        return EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY);
    }
    else
    {
        return flushResult;
    }
}

std::set<SegmentId>
SegmentCtx::GetNvramSegmentList(void)
{
    return segmentList[SegmentState::NVRAM]->GetList();
}

std::set<SegmentId>
SegmentCtx::GetVictimSegmentList(void)
{
    return segmentList[SegmentState::VICTIM]->GetList();
}

uint32_t
SegmentCtx::GetVictimSegmentCount(void)
{
    return segmentList[SegmentState::VICTIM]->GetNumSegments();
}

void
SegmentCtx::_BuildRebuildSegmentList(void)
{
    // New segment is not allocated while make rebuild segment target by design
    _BuildRebuildSegmentListFromTheList(SegmentState::SSD);
    _BuildRebuildSegmentListFromTheList(SegmentState::VICTIM);
    _BuildRebuildSegmentListFromTheList(SegmentState::NVRAM); // TODO(huijeong.kim) remove this

    _ResetSegmentIdInRebuilding();
}

void
SegmentCtx::_BuildRebuildSegmentListFromTheList(SegmentState state)
{
    while (segmentList[state]->GetNumSegments() != 0)
    {
        SegmentId targetSegment = segmentList[state]->PopSegment();
        if (targetSegment != UNMAP_SEGMENT)
        {
            POS_TRACE_DEBUG(EID(ALLOCATOR_SEGMENT_ADDED_TO_REBUILD_TARGET),
                "segment_id:{}, state:{}",
                targetSegment, segmentInfos[targetSegment].GetState());
            rebuildList->AddToList(targetSegment);
        }
    }
}

void
SegmentCtx::_ResetSegmentIdInRebuilding(void)
{
    rebuildingSegment = UNMAP_SEGMENT;
}

int
SegmentCtx::StopRebuilding(void)
{
    uint32_t remaining = rebuildList->GetNumSegments();
    POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_STOP), "num_of_remaining_segments_in_rebuild_list:{}", remaining);
    if (remaining == 0)
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY), "");
        return ERRID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY);
    }

    while (rebuildList->GetNumSegments() != 0)
    {
        SegmentId segmentId = rebuildList->PopSegment();
        if (segmentId != UNMAP_SEGMENT)
        {
            SegmentState state = segmentInfos[segmentId].GetState();
            segmentList[state]->AddToList(segmentId);
        }
    }

    _ResetSegmentIdInRebuilding();

    return _FlushRebuildSegmentList();
}

uint32_t
SegmentCtx::GetRebuildTargetSegmentCount(void)
{
    return rebuildList->GetNumSegments();
}

bool
SegmentCtx::LoadRebuildList(void)
{
    auto list = rebuildCtx->GetList();

    for (auto it = list.begin(); it != list.end(); it++)
    {
        SegmentId segmentId = *it;
        SegmentState state = segmentInfos[segmentId].GetState();
        bool removed = segmentList[state]->RemoveFromList(segmentId);
        assert(removed == true);

        rebuildList->AddToList(segmentId);
    }

    POS_TRACE_INFO(EID(ALLOCATOR_REBUILD_CTX_LOAD_SUCCESS),
        "num_of_segement_in_rebuild_list:{}", rebuildList->GetNumSegments());
    return (rebuildList->GetNumSegments() != 0);
}

std::set<SegmentId>
SegmentCtx::GetRebuildSegmentList(void)
{
    return rebuildList->GetList();
}

void
SegmentCtx::CopySegmentInfoToBufferforWBT(WBTAllocatorMetaType type, char* dstBuf)
{
    uint32_t numSegs = addrInfo->GetnumUserAreaSegments(); // for ut
    uint32_t* dst = reinterpret_cast<uint32_t*>(dstBuf);
    for (uint32_t segId = 0; segId < numSegs; segId++)
    {
        if (type == WBT_SEGMENT_VALID_COUNT)
        {
            dst[segId] = segmentInfos[segId].GetValidBlockCount();
        }
        else
        {
            dst[segId] = segmentInfos[segId].GetOccupiedStripeCount();
        }
    }
}

void
SegmentCtx::CopySegmentInfoFromBufferforWBT(WBTAllocatorMetaType type, char* srcBuf)
{
    uint32_t numSegs = addrInfo->GetnumUserAreaSegments(); // for ut
    uint32_t* src = reinterpret_cast<uint32_t*>(srcBuf);
    for (uint32_t segId = 0; segId < numSegs; segId++)
    {
        if (type == WBT_SEGMENT_VALID_COUNT)
        {
            segmentInfos[segId].SetValidBlockCount(src[segId]);
        }
        else
        {
            segmentInfos[segId].SetOccupiedStripeCount(src[segId]);
        }
    }
}

void
SegmentCtx::ValidateBlocksWithGroupId(VirtualBlks blks, int logGroupId)
{
    ValidateBlks(blks);
}

bool
SegmentCtx::InvalidateBlocksWithGroupId(VirtualBlks blks, bool isForced, int logGroupId)
{
    return InvalidateBlks(blks, isForced);
}

bool
SegmentCtx::UpdateStripeCount(StripeId lsid, int logGroupId)
{
    return UpdateOccupiedStripeCount(lsid);
}

SegmentInfo*
SegmentCtx::GetSegmentInfos(void)
{
    return segmentInfos;
}

void
SegmentCtx::ResetInfos(SegmentId segId)
{
    // TODO (dh.ihm) : need to check if there is additional implementation.
    return;
}
} // namespace pos

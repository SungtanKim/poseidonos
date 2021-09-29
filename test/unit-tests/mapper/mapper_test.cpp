#include "src/mapper/mapper.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/reversemap/reversemap_manager_mock.h"
#include "test/unit-tests/mapper/stripemap/stripemap_manager_mock.h"
#include "test/unit-tests/mapper/vsamap/vsamap_manager_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(Mapper, Init_InitAndDispose)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    EXPECT_CALL(*strMan, FlushTouchedPages).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, FlushAllMaps).WillOnce(Return(0));
    EXPECT_CALL(*strMan, WaitAllPendingIoDone).Times(2);
    EXPECT_CALL(*vsaMan, WaitAllPendingIoDone).Times(2);
    EXPECT_CALL(*strMan, Dispose).Times(1);
    EXPECT_CALL(*vsaMan, Dispose).Times(1);
    EXPECT_CALL(*revMan, Dispose).Times(1);
    mapper->Dispose();
    delete mapper;
    delete arr;
}

TEST(Mapper, TestInitAndShutdown)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);

    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    EXPECT_CALL(*strMan, WaitAllPendingIoDone).Times(1);
    EXPECT_CALL(*vsaMan, WaitAllPendingIoDone).Times(1);
    EXPECT_CALL(*strMan, Dispose).Times(1);
    EXPECT_CALL(*vsaMan, Dispose).Times(1);
    EXPECT_CALL(*revMan, Dispose).Times(1);
    mapper->Shutdown();
    delete mapper;
    delete arr;
}

TEST(Mapper, TestGetObjects)
{
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockIArrayInfo>* arrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMetaFs> mfs;
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arrayInfo, &mfs);
    IVSAMap* v = mapper->GetIVSAMap();
    IStripeMap* s = mapper->GetIStripeMap();
    IReverseMap* r = mapper->GetIReverseMap();
    IMapFlush* f = mapper->GetIMapFlush();
    IMapperWbt* w = mapper->GetIMapperWbt();
    IMapperVolumeEventHandler* ve = mapper->GetVolumeEventHandler();
    VSAMapManager* vv = mapper->GetVSAMapManager();
    StripeMapManager* ss = mapper->GetStripeMapManager();
    ReverseMapManager* rr = mapper->GetReverseMapManager();
    delete mapper;
    delete arrayInfo;
}

TEST(Mapper, TestFlushDirtyMpages)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    MpageList list;
    // when 1.
    int ret = mapper->FlushDirtyMpages(0, nullptr);
    EXPECT_EQ(0, ret);
    // when 2.
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    mapper->VolumeCreated(0, 100);
    EXPECT_CALL(*vsaMan, FlushTouchedPages).Times(1);
    ret = mapper->FlushDirtyMpages(0, nullptr);
    EXPECT_CALL(*strMan, FlushTouchedPages).Times(1);
    ret = mapper->FlushDirtyMpages(STRIPE_MAP_ID, nullptr);
    delete mapper;
    delete arr;
}

TEST(Mapper, TestFlushDirtyMpagesGiven)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    MpageList list;
    // when 1.
    int ret = mapper->FlushDirtyMpagesGiven(0, nullptr, list);
    EXPECT_EQ(0, ret);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    mapper->VolumeCreated(0, 100);
    EXPECT_CALL(*vsaMan, FlushDirtyPagesGiven).Times(1);
    ret = mapper->FlushDirtyMpagesGiven(0, nullptr, list);
    EXPECT_CALL(*strMan, FlushDirtyPagesGiven).Times(1);
    ret = mapper->FlushDirtyMpagesGiven(STRIPE_MAP_ID, nullptr, list);
    delete mapper;
    delete arr;
}

TEST(Mapper, TestStoreAll)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    int ret = mapper->StoreAll();
    // given 1.
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    mapper->VolumeCreated(0, 100);
    EXPECT_CALL(*vsaMan, FlushAllMaps).WillOnce(Return(0));
    EXPECT_CALL(*strMan, FlushTouchedPages).WillOnce(Return(0));
    // when 1.
    ret = mapper->StoreAll();
    // given 2.
    EXPECT_CALL(*strMan, FlushTouchedPages).WillOnce(Return(-1));
    EXPECT_CALL(*vsaMan, FlushAllMaps).Times(0);
    // when 2.
    ret = mapper->StoreAll();
    // given 2.
    EXPECT_CALL(*strMan, FlushTouchedPages).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, FlushAllMaps).WillOnce(Return(-1));
    // when 3.
    ret = mapper->StoreAll();

    delete mapper;
    delete arr;
}

TEST(Mapper, TestEnableInternalAccess)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    int ret = mapper->EnableInternalAccess(0);
    EXPECT_EQ(-EID(VSAMAP_LOAD_FAILURE), ret);

    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    mapper->VolumeCreated(0, 100);
    ret = mapper->EnableInternalAccess(0);
    EXPECT_EQ(0, ret);

    mapper->VolumeLoaded(1, 500);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(0));
    ret = mapper->EnableInternalAccess(1);
    EXPECT_EQ(NEED_RETRY, ret);

    mapper->VolumeLoaded(2, 500);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(-1));
    ret = mapper->EnableInternalAccess(2);
    EXPECT_EQ(-EID(VSAMAP_LOAD_FAILURE), ret);

    mapper->VolumeLoaded(3, 500);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(-1));
    ret = mapper->EnableInternalAccess(3);
    EXPECT_EQ(-EID(VSAMAP_LOAD_FAILURE), ret);
    delete mapper;
    delete arr;
}

TEST(Mapper, TestVolumeEvent)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();

    mapper->VolumeLoaded(0, 100);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(0));
    int ret = mapper->VolumeMounted(0, 100);
    EXPECT_EQ((int)POS_EVENT_ID::VOL_EVENT_OK, ret);
    ret = mapper->EnableInternalAccess(0);
    EXPECT_EQ(0, ret);
    EXPECT_CALL(*vsaMan, FlushTouchedPages).WillOnce(Return(0));
    ret = mapper->VolumeUnmounted(0, true);
    EXPECT_EQ((int)POS_EVENT_ID::VOL_EVENT_OK, ret);
    ret = mapper->VolumeUnmounted(0, true);
    EXPECT_EQ((int)POS_EVENT_ID::VOL_EVENT_OK, ret);

    ret = mapper->VolumeMounted(1, 100);
    EXPECT_EQ((int)POS_EVENT_ID::VOL_EVENT_FAIL, ret);

    mapper->VolumeLoaded(1, 100);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(-1));
    ret = mapper->VolumeMounted(1, 100);
    EXPECT_EQ((int)POS_EVENT_ID::VOL_EVENT_FAIL, ret);

    mapper->VolumeLoaded(2, 100);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(0));
    vector<int> volList;
    ret = mapper->VolumeMounted(2, 100);
    volList.push_back(2);
    ret = mapper->VolumeDetached(volList);
    EXPECT_EQ((int)POS_EVENT_ID::VOL_EVENT_OK, ret);

    delete mapper;
    delete arr;
}

TEST(Mapper, TestGetVSASetVSA)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();

    VirtualBlks v;
    VirtualBlkAddr vsa;
    vsa.stripeId = 2;
    vsa.offset = 2;
    v.startVsa = vsa;
    v.numBlks = 1;
    int ret = mapper->SetVSAsInternal(1, 0, v);
    EXPECT_EQ(-EID(VSAMAP_LOAD_FAILURE), ret);

    mapper->VolumeLoaded(1, 500);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(0));
    ret = mapper->SetVSAsInternal(1, 0, v);
    EXPECT_EQ(-EID(VSAMAP_LOAD_FAILURE), ret);

    EXPECT_CALL(*vsaMan, IsVolumeLoaded).WillOnce(Return(true));
    EXPECT_CALL(*vsaMan, SetVSAsWoCond).WillOnce(Return(0));
    ret = mapper->SetVSAsInternal(1, 0, v);
    EXPECT_EQ(0, ret);

    int retry;
    VirtualBlkAddr retblk;
    retblk = mapper->GetVSAInternal(0, 0, retry);
    EXPECT_EQ(UNMAP_VSA, retblk);

    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    mapper->VolumeCreated(0, 500);
    EXPECT_CALL(*vsaMan, GetVSAWoCond).WillOnce(Return(vsa));
    retblk = mapper->GetVSAInternal(0, 0, retry);
    EXPECT_EQ(vsa, retblk);

    retblk = mapper->GetVSAWithSyncOpen(2, 0);
    EXPECT_EQ(UNMAP_VSA, retblk);
    mapper->VolumeLoaded(2, 500);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, WaitVolumePendingIoDone).Times(1);
    EXPECT_CALL(*vsaMan, GetVSAWoCond).WillOnce(Return(vsa));
    retblk = mapper->GetVSAWithSyncOpen(2, 0);
    EXPECT_EQ(vsa, retblk);

    ret = mapper->SetVSAsWithSyncOpen(5, 0, v);
    EXPECT_EQ(-EID(VSAMAP_LOAD_FAILURE), ret);
    mapper->VolumeLoaded(5, 500);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, LoadVSAMapFile).WillOnce(Return(0));
    EXPECT_CALL(*vsaMan, WaitVolumePendingIoDone).Times(1);
    EXPECT_CALL(*vsaMan, SetVSAsWoCond).WillOnce(Return(0));
    ret = mapper->SetVSAsWithSyncOpen(5, 0, v);
    EXPECT_EQ(0, ret);

    delete mapper;
    delete arr;
}

TEST(Mapper, TestSimpleBypassFunctions)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    VirtualBlks v;
    VirtualBlkAddr vsa;
    vsa.stripeId = 2;
    vsa.offset = 2;
    v.startVsa = vsa;
    v.numBlks = 1;

    EXPECT_CALL(*vsaMan, GetDirtyVsaMapPages).Times(1);
    mapper->GetDirtyVsaMapPages(0, 0, 1);

    VsaArray a;
    EXPECT_CALL(*vsaMan, GetVSAs).Times(1);
    mapper->GetVSAs(0, 0, 1, a);

    EXPECT_CALL(*vsaMan, SetVSAs).Times(1);
    mapper->SetVSAs(0, 0, v);

    EXPECT_CALL(*vsaMan, GetRandomVSA).Times(1);
    mapper->GetRandomVSA(0);

    delete mapper;
    delete arr;
}

TEST(Mapper, TestGetNumUsedBlks)
{
    NiceMock<MockIArrayInfo>* arr = new NiceMock<MockIArrayInfo>();
    NiceMock<MockMapperAddressInfo>* addrInfo = new NiceMock<MockMapperAddressInfo>();
    NiceMock<MockVSAMapManager>* vsaMan = new NiceMock<MockVSAMapManager>();
    NiceMock<MockStripeMapManager>* strMan = new NiceMock<MockStripeMapManager>();
    NiceMock<MockReverseMapManager>* revMan = new NiceMock<MockReverseMapManager>();
    NiceMock<MockMetaFs> mfs;
    EXPECT_CALL(*arr, GetName).WillOnce(Return(""));
    Mapper* mapper = new Mapper(nullptr, vsaMan, strMan, revMan, addrInfo, arr, &mfs);
    EXPECT_CALL(*addrInfo, SetupAddressInfo).Times(1);
    EXPECT_CALL(*vsaMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*strMan, Init).WillOnce(Return(0));
    EXPECT_CALL(*revMan, Init).Times(1);
    mapper->Init();
    int ret = mapper->GetNumUsedBlks(0);
    EXPECT_EQ(0, ret);

    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(0));
    mapper->VolumeCreated(0, 100);
    EXPECT_CALL(*vsaMan, GetNumUsedBlks).WillOnce(Return(5));
    ret = mapper->GetNumUsedBlks(0);
    EXPECT_EQ(5, ret);

    mapper->VolumeLoaded(2, 100);
    EXPECT_CALL(*vsaMan, CreateVsaMapContent).WillOnce(Return(-1));
    ret = mapper->GetNumUsedBlks(2);
    EXPECT_EQ(-1, ret);

    delete mapper;
    delete arr;
}

} // namespace pos

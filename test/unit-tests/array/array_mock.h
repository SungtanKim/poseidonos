#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/array.h"

namespace pos
{
class MockArrayDebugInfo : public ArrayDebugInfo
{
public:
    using ArrayDebugInfo::ArrayDebugInfo;
};

class MockArray : public Array
{
public:
    using Array::Array;
    MOCK_METHOD(int, Import, (ArrayBuildInfo* buildInfo), (override));
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(int, Delete, (), (override));
    MOCK_METHOD(int, AddSpare, (string devName), (override));
    MOCK_METHOD(int, RemoveSpare, (string devName), (override));
    MOCK_METHOD(int, ReplaceDevice, (string devName), (override));
    MOCK_METHOD(int, Rebuild, (), (override));
    MOCK_METHOD(int, DetachDevice, (IArrayDevice* dev), (override));
    MOCK_METHOD(void, MountDone, (), (override));
    MOCK_METHOD(int, CheckUnmountable, (), (override));
    MOCK_METHOD(string, Serialize, (), (override));

    MOCK_METHOD(string, GetName, (), (override));
    MOCK_METHOD(uint32_t, GetIndex, (), (override));
    MOCK_METHOD(string, GetUniqueId, (), (override));
    MOCK_METHOD(string, GetCreateDatetime, (), (override));
    MOCK_METHOD(string, GetUpdateDatetime, (), (override));
    MOCK_METHOD(string, GetMetaRaidType, (), (override));
    MOCK_METHOD(string, GetDataRaidType, (), (override));
    MOCK_METHOD(ArrayStateType, GetState, (), (override));
    MOCK_METHOD(StateContext*, GetStateCtx, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildingProgress, (), (override));
    MOCK_METHOD(bool, IsWriteThroughEnabled, (), (override));
    MOCK_METHOD(vector<IArrayDevice*>, GetDevices, (ArrayDeviceType type), (override));
    MOCK_METHOD(const PartitionLogicalSize*, GetSizeInfo, (PartitionType type), (override));

    MOCK_METHOD(IArrayDevice*, FindDevice, (string devSn), (override));
    MOCK_METHOD(void, InvokeRebuild, (vector<IArrayDevice*> targets, bool isResume, bool force), (override));
    MOCK_METHOD(bool, TriggerRebuild, (vector<IArrayDevice*> targets), (override));
    MOCK_METHOD(bool, ResumeRebuild, (vector<IArrayDevice*> targets), (override));
    MOCK_METHOD(void, DoRebuildAsync, (vector<IArrayDevice*> dst, vector<IArrayDevice*> src, RebuildTypeEnum rt), (override));
    MOCK_METHOD(void, SetPreferences, (bool isWT), (override));
    MOCK_METHOD(void, SetTargetAddress, (string targetAddress), (override));
    MOCK_METHOD(string, GetTargetAddress, (), (override));
};

} // namespace pos

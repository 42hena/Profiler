#pragma once

#include <Windows.h>

#define PROFILE_BEGIN(name) ProfilerUtil::ProfileBegin(name)
#define PROFILE_END(name) ProfilerUtil::ProfileEnd(name)
#define PROFILE_FUNCTION ProfileMeasure func(__FUNCTION__)

#define CONCAT_NAME(x, y) x##y
#define MAKE_VAR_NAME(x, y) CONCAT_NAME(x, y)
#define PROFILE_SCOPE(name) ProfileMeasure scope(name)
#define PROFILE_SCOPE_EX(name) ProfileMeasure MAKE_VAR_NAME(profilerObj, __LINE__)(name)

#define TAG_NAME_SAME 0

enum eProfileLimit
{
    PROFILE_LIMIT_MIN_MAX_ARRAY = 2,
    PROFILE_LIMIT_ARRAY_LENGTH = 5,
    PROFILE_LIMIT_STRING_MAX = 32
};






struct ProfilerData
{
    bool bUse;
    char name[PROFILE_LIMIT_STRING_MAX];
    double totalTime;
    LARGE_INTEGER startTime;
    double maxTimes[PROFILE_LIMIT_MIN_MAX_ARRAY];
    double minTimes[PROFILE_LIMIT_MIN_MAX_ARRAY];
    long long tagCallCount;
};






class ProfileMeasure
{
public:
    ProfileMeasure(const char* tagName);
    ~ProfileMeasure();

private:
    const char* _tagName;
};






namespace ProfilerUtil
{
    void ProfileBegin(const char* tagName);
    void ProfileEnd(const char* tagName);
    void ProfileDataOutText();
    void ProfileClear();
}

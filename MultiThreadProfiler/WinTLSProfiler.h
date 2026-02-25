#pragma once

#include "PriorityQueue.h"
#include <Windows.h>



#define WINPRO_BEGIN(TagName)	WinTLSProfilerUtil::ProfileBegin(TagName)
#define WINPRO_END(TagName)	    WinTLSProfilerUtil::ProfileEnd(TagName)

enum eProfilerLimit
{
    eProfilerLimit_ProfileArrayCount = 10,
    eProfilerLimit_ThreadCount = 16,
};






struct ProfilerData
{
public:
    ProfilerData();

public:
    bool            flag;
    char            name[50];
    long long       runningTime;
    LARGE_INTEGER   beginTime;
    PriorityQueue<long long, std::greater<long long>> maxTime;
    PriorityQueue<long long, std::less<long long>> minTime;
    //long long       maxTime[2];
    //long long       minTime[2];
    long long       callCount;
    int             arrayIndex;
};

class WinThreadLocalProfiler
{
public:
    WinThreadLocalProfiler();
    ~WinThreadLocalProfiler();

public:
    WinThreadLocalProfiler(const WinThreadLocalProfiler&)               = delete;
    WinThreadLocalProfiler& operator=(const WinThreadLocalProfiler&)    = delete;
    WinThreadLocalProfiler(WinThreadLocalProfiler&&)                    = delete;
    WinThreadLocalProfiler& operator=(WinThreadLocalProfiler&&)         = delete;


public:
    bool GetFlag(int idx) const;
    long long GetCallCount(int idx) const;
    const char* GetTagName(int idx) const;


    void UseData(int idx, const char* tagName);
    void StartRecord(int idx);

    __int64 GetStartTime(int idx);

    void AddRunTime(int idx, __int64 operatingTime);
    void ChangeMaxValue(int idx, __int64 operatingTime);
    void ChangeMinValue(int idx, __int64 operatingTime);

    long long GetMaxValue(int idx);
    long long GetMinValue(int idx);
    long long GetTotalMaxValue(int idx);
    long long GetTotalMinValue(int idx);
    long long GetTotalRuntime(int idx)
    {
        return profileData[idx].runningTime;
    }
    DWORD GetThreadID() { return _threadId; }
    void SetThreadID(unsigned int threadId) { _threadId = threadId;}

private:
    ProfilerData profileData[eProfilerLimit_ProfileArrayCount];
    unsigned int _threadId;
};

class WinTLSProfileManager
{
public:
    WinTLSProfileManager();
    ~WinTLSProfileManager();

public:
    WinTLSProfileManager(const WinTLSProfileManager&)             = delete;
    WinTLSProfileManager& operator=(const WinTLSProfileManager&)  = delete;
    WinTLSProfileManager(WinTLSProfileManager&&)                  = delete;
    WinTLSProfileManager& operator=(WinTLSProfileManager&&)       = delete;

public:
    static WinTLSProfileManager& Instance();

public:
    int GetUseProfileIndex() { return index;}
    DWORD GetTlsIndex();
    DWORD GetProfileIndex();
    WinThreadLocalProfiler& GetThread(int index)
    {
        return threadArray[index];
    }
    WinThreadLocalProfiler* GetProfile();

private:
    WinThreadLocalProfiler      threadArray[eProfilerLimit_ThreadCount];
    DWORD                       _tlsIndex;
    alignas(64) long            index;
};

// #define MAX_PROFILE_SIZE 10



class WinTLSProfilerMeasure
{
public:
    WinTLSProfilerMeasure(const char* name);
    ~WinTLSProfilerMeasure();

public:
    WinTLSProfilerMeasure(const WinTLSProfilerMeasure&) = delete;
    WinTLSProfilerMeasure& operator=(const WinTLSProfilerMeasure&) = delete;
    WinTLSProfilerMeasure(WinTLSProfilerMeasure&&) = delete;
    WinTLSProfilerMeasure& operator=(WinTLSProfilerMeasure&&) = delete;

private:
    const char* _tagName;
};


namespace WinTLSProfilerUtil
{
    void ProfileBegin(const char* name);
    void ProfileEnd(const char* name);
    void ProfileDataOutToText();
}

#include <iostream>
#include <Windows.h>

#include "WinTLSProfiler.h"


//ProfilerData::ProfilerData()
//    : flag(false),
//    runningTime(0),
//    beginTime{ 0, 0 },
//    maxTime{ 0, 0 },
//    minTime{ 0x7fffffffffffffff, 0x7fffffffffffffff },
//    callCount(0),
//    arrayIndex(0)
//{
//    name[0] = 0;
//#if defined(MY_DEBUG) && defined(SPROFILER_DATA_DEBUG)
//#endif
//}






WinThreadLocalProfiler::WinThreadLocalProfiler()
    : _threadId(0)
{
}

WinThreadLocalProfiler::~WinThreadLocalProfiler()
{
}

bool WinThreadLocalProfiler::GetFlag(int idx) const
{
    return profileData[idx].flag;
}

long long WinThreadLocalProfiler::GetCallCount(int idx) const
{
    return profileData[idx].callCount;
}

const char* WinThreadLocalProfiler::GetTagName(int idx) const
{
    return profileData[idx].name;
}

void WinThreadLocalProfiler::UseData(int idx, const char* tagName)
{
    strcpy_s(profileData[idx].name, tagName);
    profileData[idx].flag = true;

    //wcscpy_s(profileThread->profileData[destIndex].name, name);
    //profileThread->profileData[destIndex].flag = true;
}

void WinThreadLocalProfiler::StartRecord(int idx)
{
    /*QueryPerformanceCounter(&profileThread->profileData[destIndex].beginTime);
    profileThread->profileData[destIndex].callCount++;*/
    QueryPerformanceCounter(&profileData[idx].beginTime);
    profileData[idx].callCount++;
}

long long WinThreadLocalProfiler::GetStartTime(int idx)
{
    return profileData[idx].beginTime.QuadPart;
}

void WinThreadLocalProfiler::AddRunTime(int idx, long long operatingTime)
{
    profileData[idx].runningTime += operatingTime;
}

void WinThreadLocalProfiler::ChangeMaxValue(int idx, long long operatingTime)
{
    if (profileData[idx].maxTime.Size() < eProfilerLimit_ProfileArrayCount)
    {
        profileData[idx].maxTime.Push(operatingTime);
    }
    else
    {
        long long value;
        profileData[idx].maxTime.Pop(value);
        profileData[idx].maxTime.Push(operatingTime);
    }
}

void WinThreadLocalProfiler::ChangeMinValue(int idx, long long operatingTime)
{
    if (profileData[idx].minTime.Size() < eProfilerLimit_ProfileArrayCount)
    {
        profileData[idx].minTime.Push(operatingTime);
    }
    else
    {
        long long value;
        profileData[idx].minTime.Pop(value);
        profileData[idx].minTime.Push(operatingTime);
    }
}

long long WinThreadLocalProfiler::GetMaxValue(int idx)
{
    if (profileData[idx].maxTime.Empty())
    {
        return 0;
    }
    long long maxTimeValue;
    profileData[idx].maxTime.Top(maxTimeValue);
    return maxTimeValue;
}

long long WinThreadLocalProfiler::GetMinValue(int idx)
{
    if (profileData[idx].minTime.Empty())
    {
        return 0;
    }
    long long minTimeValue;
    profileData[idx].minTime.Top(minTimeValue);
    return minTimeValue;
}

long long WinThreadLocalProfiler::GetTotalMaxValue(int idx)
{
    long long totalTime;
    long long maxTimeValue;

    totalTime = 0;
    for (int i = 0; i < eProfilerLimit_ProfileArrayCount; ++i)
    {
        if (profileData[idx].maxTime.Pop(maxTimeValue) == false)
        {
            break;
        }
        totalTime += maxTimeValue;
    }
    return totalTime;
}

long long WinThreadLocalProfiler::GetTotalMinValue(int idx)
{
    long long totalTime;
    long long minTimeValue;

    totalTime = 0;
    for (int i = 0; i < eProfilerLimit_ProfileArrayCount; ++i)
    {
        if (profileData[idx].minTime.Pop(minTimeValue) == false)
        {
            break;
        }
        totalTime += minTimeValue;
    }
    return totalTime;
}

WinTLSProfileManager::WinTLSProfileManager()
    : index(-1)
{
    _tlsIndex = TlsAlloc();

    //TlsAlloc이 실패 시 에러 처리
    if (_tlsIndex == TLS_OUT_OF_INDEXES)
    {
        printf("In [WinTLSProfileManager], tlsIndex is [TLS_OUT_OF_INDEXES]\n");
        __debugbreak();
    }
    
    printf("tlsIndex is %d\n", _tlsIndex);
}

WinTLSProfileManager::~WinTLSProfileManager()
{
    TlsFree(_tlsIndex);
}

WinTLSProfileManager& WinTLSProfileManager::Instance()
{
    static WinTLSProfileManager instance;

    return instance;
}

DWORD WinTLSProfileManager::GetTlsIndex()
{
    return _tlsIndex;
}

DWORD WinTLSProfileManager::GetProfileIndex()
{
    int prevIndex;
    
    prevIndex = InterlockedAdd(&index, 1);
    threadArray[prevIndex].SetThreadID(GetCurrentThreadId());
    return prevIndex;
}

WinThreadLocalProfiler* WinTLSProfileManager::GetProfile()
{
    return &threadArray[GetProfileIndex()];
}


WinTLSProfilerMeasure::WinTLSProfilerMeasure(const char* name)
    : _tagName(name)
{
    WinTLSProfilerUtil::ProfileBegin(name);
}

WinTLSProfilerMeasure::~WinTLSProfilerMeasure()
{
    WinTLSProfilerUtil::ProfileEnd(_tagName);
}


void WinTLSProfilerUtil::ProfileBegin(const char* tagName)
{
    int profileIndex;
    bool flag;
    DWORD tlsIndex;
    LPVOID ptr;
    WinThreadLocalProfiler* profiler;

    flag = false;
    tlsIndex = WinTLSProfileManager::Instance().GetTlsIndex();

    ptr = TlsGetValue(tlsIndex);
    if (ptr == nullptr)
    {
        profiler = WinTLSProfileManager::Instance().GetProfile();
        TlsSetValue(tlsIndex, profiler);
    }
    else
    {
        profiler = reinterpret_cast<WinThreadLocalProfiler*>(ptr);
    }

    // Find empty array
    for (profileIndex = 0; profileIndex < eProfilerLimit_ProfileArrayCount; profileIndex += 1)
    {
        // if ( (profileThread->profileData[i].flag == false) || !wcscmp(name, profileThread->profileData[i].name))
        if ((profiler->GetFlag(profileIndex) == false))
        {
            break;
        }
        if (strcmp(tagName, profiler->GetTagName(profileIndex)) == false)
        {
            break;
        }
    }

    if (profileIndex == eProfilerLimit_ProfileArrayCount)
    {
        __debugbreak();
        return;
    }

    // 첫 시도
    if (profiler->GetFlag(profileIndex) == false)
    {
        profiler->UseData(profileIndex, tagName);
    }

    // Start record time and call count
    profiler->StartRecord(profileIndex);
}

void WinTLSProfilerUtil::ProfileEnd(const char* name)
{
    int profileIndex;
    long long operatingTime;
    LARGE_INTEGER endTime;
    WinThreadLocalProfiler* profileThread;
    DWORD tlsIndex;
    

    // 시간 측정 완료
    QueryPerformanceCounter(&endTime);

    // 프로파일러의 TLS 값 획득
    tlsIndex = WinTLSProfileManager::Instance().GetTlsIndex();
    if (tlsIndex < 0)
    {
        __debugbreak();
        return;
    }

    void* LocalProfilerAddress;
    LocalProfilerAddress = TlsGetValue(tlsIndex);
    if (LocalProfilerAddress == nullptr)
    {
        wprintf(L"ProfileEnd");
        return;
    }
    profileThread = reinterpret_cast<WinThreadLocalProfiler*>(LocalProfilerAddress);

    // Find dest time
    profileIndex = -1;
    for (profileIndex = 0; profileIndex < eProfilerLimit_ProfileArrayCount; profileIndex += 1)
    {
        // strcmp는 같은 경우 0이다.
        if (strcmp(name, profileThread->GetTagName(profileIndex)) == false)
        {
            break;
        }
    }

    if (profileIndex == eProfilerLimit_ProfileArrayCount)
    {
        __debugbreak();
        return;
    }

    operatingTime = endTime.QuadPart - profileThread->GetStartTime(profileIndex);
    // Add Running Time
    profileThread->AddRunTime(profileIndex, operatingTime);

    // Change Max
    profileThread->ChangeMaxValue(profileIndex, operatingTime);

    // Change Min
    profileThread->ChangeMinValue(profileIndex, operatingTime);
}

void WinTLSProfilerUtil::ProfileDataOutToText()
{
    char buf[256];
    FILE* fp;
    LARGE_INTEGER freq;
    SYSTEMTIME tm;
    int errCode;
    int profileArrayIndex;

    QueryPerformanceFrequency(&freq);

    GetLocalTime(&tm);
    sprintf_s(buf, 256, "%4d_%02d_%02d_%02d_%02d_profile_log.txt", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute);


    errCode = fopen_s(&fp, buf, "ab+");
    if (errCode)
    {
        errCode = GetLastError();
        wprintf(L"ProfileDataOutText Fail %d\n", errCode);
        return;
    }

    int useProfileIndex = WinTLSProfileManager::Instance().GetUseProfileIndex();
    for (int idx = 0; idx <= useProfileIndex; ++idx)
    {
        sprintf_s(buf, 200, "THREAD|        Name        |       Average      |         Min        |          Max       |        Call        | \n");
        fwrite(buf, 1, strlen(buf), fp);

        WinThreadLocalProfiler& now = WinTLSProfileManager::Instance().GetThread(idx);
        for (int profileArrayIndex = 0; profileArrayIndex < eProfilerLimit_ProfileArrayCount; profileArrayIndex += 1)
        {
            if (now.GetFlag(profileArrayIndex) == false)
            {
                break;
                
            }
            
            long long maxValue = now.GetMaxValue(profileArrayIndex);
            long long minValue = now.GetMinValue(profileArrayIndex);
            long long callCount = now.GetCallCount(profileArrayIndex);
            if (callCount > 20)
            {
                sprintf_s(buf, 200, "%6d|%-20s|%18.6lfms|%18.6lfms|%18.6lfms|%20lld|\n",
                    now.GetThreadID(),
                    now.GetTagName(profileArrayIndex),
                    (double)(now.GetTotalRuntime(profileArrayIndex)
                        - now.GetTotalMaxValue(profileArrayIndex)
                        - now.GetTotalMinValue(profileArrayIndex)) / (now.GetCallCount(profileArrayIndex) - 20) / freq.QuadPart * 1000,
                    minValue / (double)freq.QuadPart * 1000,
                    maxValue / (double)freq.QuadPart * 1000,
                    callCount);
                fwrite(buf, 1, strlen(buf), fp);
            }
            else
            {
                sprintf_s(buf, 200, "%6d|%-20s|%18.6lfms|%18.6lfms|%18.6lfms|%20lld|\n",
                    now.GetThreadID(),
                    now.GetTagName(profileArrayIndex),
                    (double)(now.GetTotalRuntime(profileArrayIndex)) / (now.GetCallCount(profileArrayIndex)) / freq.QuadPart * 1000,
                    minValue / (double)freq.QuadPart * 1000,
                    maxValue / (double)freq.QuadPart * 1000,
                    callCount);
                fwrite(buf, 1, strlen(buf), fp);
            }
        }
    }
    fclose(fp);
}

ProfilerData::ProfilerData()
    :maxTime(10)
    ,minTime(10)
    , flag(false)
    , runningTime(0)
    , callCount(0)
    
{
}

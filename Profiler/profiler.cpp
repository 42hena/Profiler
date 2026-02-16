#include "Profiler.h"

#include <iostream>
#include <chrono>
#include <Windows.h>

LARGE_INTEGER global_start;
LARGE_INTEGER global_end;

ProfilerData global_profilerData[PROFILE_LIMIT_ARRAY_LENGTH];

void ProfilerUtil::ProfileBegin(const char* tagName)
{
    int profileIndex;
    int minTimeIndex;

    // 저장할 위치 검색
    for (profileIndex = 0; profileIndex < PROFILE_LIMIT_ARRAY_LENGTH; profileIndex += 1)
    {
        // 미사용 케이스
        if ((global_profilerData[profileIndex].bUse == false))
        {
            // min 값 초기화
            for (minTimeIndex = 0; minTimeIndex < PROFILE_LIMIT_MIN_MAX_ARRAY; minTimeIndex += 1)
            {
                global_profilerData[profileIndex].minTimes[minTimeIndex] = DBL_MAX;
            }

            global_profilerData[profileIndex].bUse = true;
            strcpy_s(global_profilerData[profileIndex].name, PROFILE_LIMIT_STRING_MAX, tagName);
            break;
        }

        // tagName이 동일한 경우
        if (strcmp(tagName, global_profilerData[profileIndex].name) == TAG_NAME_SAME)
        {
            break;
        }
    }

    if (profileIndex > PROFILE_LIMIT_ARRAY_LENGTH)
    {
        std::cout << "In [ProfileBegin] Can't Find right profileIndex\n";
        __debugbreak();
        return;
    }

    // 정상적인 경우 프로파일링 시작
    
    QueryPerformanceCounter(&global_start);
    global_profilerData[profileIndex].startTime = global_start;
    global_profilerData[profileIndex].tagCallCount += 1;
}

void ProfilerUtil::ProfileEnd(const char* tagName)
{
    int profileIndex;
    double time;
    double temporaryTime;
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    LARGE_INTEGER frequency;

    // 현재 시간 및 주파수 측정
    QueryPerformanceCounter(&end);
    QueryPerformanceFrequency(&frequency);

    // 저장할 위치 검색 (사용자님 코드 로직 유지)
    for (profileIndex = 0; profileIndex < PROFILE_LIMIT_ARRAY_LENGTH; profileIndex += 1)
    {
        if (strcmp(tagName, global_profilerData[profileIndex].name) == TAG_NAME_SAME)
        {
            break;
        }
    }

    if (profileIndex == PROFILE_LIMIT_ARRAY_LENGTH)
    {
        std::cout << "In [ProfileEnd] Can't Find right profileIndex\n";
        __debugbreak();
        return;
    }

    start = global_profilerData[profileIndex].startTime;

    // 틱 차이를 나노초(ns)로 변환 (10억을 곱함)
    // 공식: (diff * 1,000,000,000) / frequency
    time = (double)(end.QuadPart - start.QuadPart) * 1000000000.0 / (double)frequency.QuadPart;

    // 총 걸린 시간 갱신 (단위: ns)
    global_profilerData[profileIndex].totalTime += time;

    // 최대/최소 오차 시간 갱신 로직 (사용자님 코드 로직 유지)
    int timeIndex;
    for (timeIndex = 0; timeIndex < 2; timeIndex += 1)
    {
        if (time > global_profilerData[profileIndex].maxTimes[timeIndex])
        {
            temporaryTime = global_profilerData[profileIndex].maxTimes[timeIndex];
            global_profilerData[profileIndex].maxTimes[timeIndex] = time;
            time = temporaryTime;
        }
    }

    // 시간 변수 초기화 후 최소값 갱신
    time = (double)(end.QuadPart - start.QuadPart) * 1000000000.0 / (double)frequency.QuadPart;
    for (timeIndex = 0; timeIndex < 2; timeIndex += 1)
    {
        if (time < global_profilerData[profileIndex].minTimes[timeIndex])
        {
            temporaryTime = global_profilerData[profileIndex].minTimes[timeIndex];
            global_profilerData[profileIndex].minTimes[timeIndex] = time;
            time = temporaryTime;
        }
    }
}

void ProfilerUtil::ProfileDataOutText()
{
    FILE* fp;
    char buf[256];
    time_t nowTime;
    tm currentTime;
    int i;
    int errorCode;

    auto now = std::chrono::system_clock::now();
    nowTime = std::chrono::system_clock::to_time_t(now);
    localtime_s(&currentTime, &nowTime);
    sprintf_s(buf, 256, "%d-%d-%d_%d-%d-%d.txt", currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday,
        currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec);
    errorCode = fopen_s(&fp, buf, "wb+");
    if (errorCode != 0)
    {
        errorCode = GetLastError();
        printf("[In ProfileDataOutText Function] Error code:%d\n", errorCode);
        return;
    }

    //sprintf_s(buf, 256, "Name      |Average   |Min       |Max       |Call      |\n");
    sprintf_s(buf, 256, "%-20s | %-15s | %-15s | %-15s | %-10s |\n",
        "Name", "Average", "Min", "Max", "Call");
    fwrite(buf, 1, strlen(buf), fp);

    // 모든 배열 순회 및 파일로 출력
    for (i = 0; i < PROFILE_LIMIT_ARRAY_LENGTH; ++i)
    {
        if (global_profilerData[i].bUse)
        {
            // 최댓값 2개, 최솟값 2개를 제외한 평균 계산 (사용자님 로직)
            double avgTime;
            if (global_profilerData[i].tagCallCount > PROFILE_LIMIT_MIN_MAX_ARRAY * 2)
            {
                avgTime = (global_profilerData[i].totalTime
                    - global_profilerData[i].minTimes[0] - global_profilerData[i].minTimes[1]
                    - global_profilerData[i].maxTimes[0] - global_profilerData[i].maxTimes[1])
                    / (global_profilerData[i].tagCallCount - 4);

                sprintf_s(buf, 256, "%-20s | %-12.2f ns | %-12.2f ns | %-12.2f ns | %-10lld |\n",
                    global_profilerData[i].name, avgTime,
                    global_profilerData[i].minTimes[0], global_profilerData[i].maxTimes[0],
                    global_profilerData[i].tagCallCount - 4);
            }
            else
            {
                avgTime = global_profilerData[i].totalTime / global_profilerData[i].tagCallCount;
                sprintf_s(buf, 256, "%-20s | %-12.2f ns | %-12.2f ns | %-12.2f ns | %-10lld |\n",
                    global_profilerData[i].name, avgTime,
                    global_profilerData[i].minTimes[0], global_profilerData[i].maxTimes[0],
                    global_profilerData[i].tagCallCount);
            }
            fwrite(buf, 1, strlen(buf), fp);
        }
    }
    fclose(fp);
}

void ProfilerUtil::ProfileClear()
{
    for (int i = 0; i < PROFILE_LIMIT_ARRAY_LENGTH; ++i)
    {
        global_profilerData[i].bUse = false;
        memset(global_profilerData[i].name, 0, PROFILE_LIMIT_STRING_MAX);
        global_profilerData[i].totalTime = 0.0;
        global_profilerData[i].tagCallCount = 0;
        global_profilerData[i].startTime.QuadPart = 0;

        for (int j = 0; j < PROFILE_LIMIT_MIN_MAX_ARRAY; ++j)
        {
            global_profilerData[i].maxTimes[j] = 0.0;
            // 중요: 최솟값은 아주 큰 값으로 초기화해야 다음 측정값이 저장됩니다.
            global_profilerData[i].minTimes[j] = DBL_MAX;
        }
    }
}





ProfileMeasure::ProfileMeasure(const char* tagName)
    : _tagName(tagName)
{
    ProfilerUtil::ProfileBegin(_tagName);
}

ProfileMeasure::~ProfileMeasure()
{
    ProfilerUtil::ProfileEnd(_tagName);
}

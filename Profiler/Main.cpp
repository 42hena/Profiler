#include <iostream>
#include <chrono>
#include "Profiler.h"

void Test()
{
    PROFILE_FUNCTION;
}

int main()
{
    Test();
    for (int i = 0; i < 5; ++i)
    {
        PROFILE_SCOPE_EX("QWER");
        PROFILE_BEGIN("Call_Sleep");
        Sleep(1000);
        PROFILE_END("Call_Sleep");
    }

    ProfilerUtil::ProfileDataOutText();
}

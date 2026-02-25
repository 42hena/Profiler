
#include <iostream>
#include <Windows.h>

#include "WinTLSProfiler.h"

#include <thread>


void T1()
{
    for (int i = 0; i < 10; ++i)
    {

        WinTLSProfilerMeasure m("T1");
        Sleep(1000);
    }
}

void T2()
{

    for (int i = 0; i < 10; ++i)
    {

        WinTLSProfilerMeasure m("T2");
        Sleep(2000);
    }
}



int main()
{


    std::thread t[10];
    for (int i = 0 ; i < 5 ; ++i)
        t[i] = std::thread(T1);

    for (int i = 5; i < 10; ++i)
        t[i] = std::thread(T2);


    for (int i = 0; i < 10; ++i)
        t[i].join();
    /*{
        for (int i = 0; i < 10; ++i)
        {

            WinTLSProfilerMeasure m("T1");
            Sleep(1000);
        }
    }*/
    WinTLSProfilerUtil::ProfileDataOutToText();
}
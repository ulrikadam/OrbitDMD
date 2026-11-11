//
// latency.cpp
//
#include "latency.h"
#include <circle/sched/scheduler.h>
#include <circle/timer.h>

unsigned CLatencyProbe::s_nWindowMax[Windows] = {0};
unsigned CLatencyProbe::s_nCurrent = 0;

CLatencyProbe::CLatencyProbe (void)
{
	SetName ("latence");
}

void CLatencyProbe::Run (void)
{
	u64 nWindowStart = CTimer::GetClockTicks64 ();

	while (TRUE)
	{
		u64 nBefore = CTimer::GetClockTicks64 ();
		CScheduler::Get ()->usSleep (SleepMicros);
		u64 nAfter = CTimer::GetClockTicks64 ();

		u64 nElapsed = nAfter - nBefore;
		unsigned nLateMs = nElapsed > SleepMicros ? (unsigned) ((nElapsed - SleepMicros) / 1000) : 0;

		if (nLateMs > s_nWindowMax[s_nCurrent])
		{
			s_nWindowMax[s_nCurrent] = nLateMs;
		}

		if (nAfter - nWindowStart >= WindowSeconds * 1000000ULL)
		{
			s_nCurrent = (s_nCurrent + 1) % Windows;
			s_nWindowMax[s_nCurrent] = 0;
			nWindowStart = nAfter;
		}
	}
}

unsigned CLatencyProbe::GetRecentMs (void)
{
	unsigned nPrevious = (s_nCurrent + Windows - 1) % Windows;
	unsigned a = s_nWindowMax[s_nCurrent], b = s_nWindowMax[nPrevious];

	return a > b ? a : b;
}

unsigned CLatencyProbe::GetMaxMs (void)
{
	unsigned nMax = 0;
	for (unsigned i = 0; i < Windows; i++)
	{
		if (s_nWindowMax[i] > nMax)
		{
			nMax = s_nWindowMax[i];
		}
	}

	return nMax;
}

//
//  FPSMoveClock.cpp
//  Controller Clock
//   Used to provide update time deltas in seconds between updates for a controller
//   in a thread safe manner.
//
//  Created by Brendan Walker on 2015-06-25.
//
#include "PSMovePrivatePCH.h"
#include "FPSMoveClock.h"
//#include "DateTime.h"
#include "FPSMove.h"

static const float MICROSECONDS_PER_MILLISECOND = 1000.f;

void FPSMoveControllerClock::Initialize()
{
	CurrentTimestamp = FPlatformTime::Cycles();
	TimeDeltaInSeconds = 0;
}

void FPSMoveControllerClock::Update()
{
	const uint32 NewTimeStamp = FPlatformTime::Cycles();

	const double TickDelta = static_cast<double>(NewTimeStamp - CurrentTimestamp);

	// Do the division to convert the tick delta to seconds and then truncate to a float
	TimeDeltaInSeconds = FPlatformTime::ToSeconds(TickDelta);

	// Remember the new timestamp for the next query
	CurrentTimestamp = NewTimeStamp;
}

FPSMoveHitchWatchdog::FPSMoveHitchWatchdog(
	const TCHAR *blockName,
	float timeout) :
	m_blockName(blockName),
	m_timeout(timeout),
	m_startTimestamp(FPlatformTime::Cycles())
{
}

FPSMoveHitchWatchdog::~FPSMoveHitchWatchdog()
{
	uint32 endTimeStamp = FPlatformTime::Cycles();
	float durationMicrosecond = FPlatformTime::ToMilliseconds(endTimeStamp - m_startTimestamp) * MICROSECONDS_PER_MILLISECOND;

	if (durationMicrosecond >= m_timeout)
	{
		UE_LOG(LogPSMove, Warning, TEXT("FPSMoveHitchWatchdog: HITCH DETECTED(%s)! Section took %.1fus (>=%.1fus)"), 
			m_blockName, durationMicrosecond, m_timeout);
	}
}
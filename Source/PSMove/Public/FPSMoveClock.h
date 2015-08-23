//
//  FPSMoveClock.h
//  Controller Clock
//   Used to provide update time deltas in seconds between updates for a controller
//   in a thread safe manner.
//
//  Created by Brendan Walker on 2015-06-25.
//
#pragma once

struct FPSMoveControllerClock
{
	uint32 CurrentTimestamp;
	float TimeDeltaInSeconds;

	void Initialize();
	void Update();
};

class FPSMoveHitchWatchdog
{
public:
	FPSMoveHitchWatchdog(const TCHAR *blockName, float microseconds_timeout);
	~FPSMoveHitchWatchdog();

private:
	const TCHAR *m_blockName;
	const float m_timeout;
	uint32 m_startTimestamp;
};
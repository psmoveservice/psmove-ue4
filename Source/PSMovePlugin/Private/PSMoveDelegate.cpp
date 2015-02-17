//
//  PSMovePluginDelegate.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-17.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//

#include "PSMovePluginPrivatePCH.h"
#include "PSMoveDelegate.h"
#include "IPSMovePlugin.h" // Necessary to call its PSMoveSetDelegate and PSMoveTick

// Empty event emitters.

    //void PSMovePluginDelegate::PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FQuat quat){}
    //void PSMovePluginDelegate::PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FRotator rot){}

    // Required functions
void PSMoveDelegate::PSMoveStartup()
{
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().SetDelegate((PSMoveDelegate*)this);
    }
}
void PSMoveDelegate::PSMoveShutdown()
{
    
}
void PSMoveDelegate::PSMoveTick(float DeltaTime)
{
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().PSMoveTick(DeltaTime);
    }
}
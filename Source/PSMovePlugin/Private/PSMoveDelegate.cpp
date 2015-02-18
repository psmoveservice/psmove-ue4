//
//  PSMovePluginDelegate.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-17.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//

#include "PSMovePluginPrivatePCH.h" // includes PSMoveDelegate.h
#include "IPSMovePlugin.h" // Necessary to call its PSMoveSetDelegate and PSMoveTick

// Empty event emitters.
void PSMoveDelegate::PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FVector pos, FQuat quat){}

    // Required functions
void PSMoveDelegate::PSMoveStartup()
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("PSMoveDelegate::PSMoveStartup"));
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().SetDelegate((PSMoveDelegate*)this);
    }
}
void PSMoveDelegate::PSMoveShutdown()
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("PSMoveDelegate::PSMoveShutdown"));
}
void PSMoveDelegate::PSMoveTick(float DeltaTime)
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("PSMoveDelegate::PSMoveTick"));
    if (IPSMovePlugin::IsAvailable())
    {
        IPSMovePlugin::Get().PSMoveTick(DeltaTime);
    }
}
//
//  PSMovePlayerController.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-17.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#include "PSMovePluginPrivatePCH.h"
#include "PSMovePlayerController.h"

APSMovePlayerController::APSMovePlayerController(const class FObjectInitializer& PCIP)
    : Super(PCIP)
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("APSMovePlayerController::APSMovePlayerController"));
    PrimaryActorTick.bCanEverTick = true;
}

void APSMovePlayerController::PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FVector pos, FQuat quat)
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("APSMovePlayerController::PSMoveOn6DOFData"));
}

/** Mandatory overrides */
void APSMovePlayerController::BeginPlay()
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("APSMovePlayerController::BeginPlay"));
    Super::BeginPlay();
    PSMoveStartup();
}

void APSMovePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("APSMovePlayerController::EndPlay"));
    Super::EndPlay(EndPlayReason);
    PSMoveShutdown();
}

void APSMovePlayerController::Tick(float DeltaTime)
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("APSMovePlayerController::Tick"));
    Super::Tick(DeltaTime);
    PSMoveTick(DeltaTime);
}
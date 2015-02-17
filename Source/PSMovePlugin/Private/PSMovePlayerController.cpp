//
//  PSMovePlayerController.cpp
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-17.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#include "PSMovePluginPrivatePCH.h"
#include "PSMoveDelegate.h"
#include "PSMovePlayerController.h"

APSMovePlayerController::APSMovePlayerController(const class FObjectInitializer& PCIP)
    : Super(PCIP)
{
    PrimaryActorTick.bCanEverTick = true;
}

/** Mandatory overrides */
void APSMovePlayerController::BeginPlay()
{
    Super::BeginPlay();
    PSMoveStartup();
}

void APSMovePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    PSMoveShutdown();
}

void APSMovePlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    PSMoveTick(DeltaTime);
}
//
//  PSMovePlayerController.h
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-17.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#pragma once

#include "GameFramework/PlayerController.h"  // Inherited from here
#include "PSMoveDelegate.h"  // Inherited from here too
#include "PSMovePlayerController.generated.h"

UCLASS()
class APSMovePlayerController : public APlayerController, public PSMoveDelegate
{
    GENERATED_UCLASS_BODY()

    void PSMoveOn6DOFData(int32 deviceId, uint64 timestamp, FVector pos, FQuat quat) override;

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;
};
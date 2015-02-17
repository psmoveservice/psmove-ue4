//
//  PSMovePlayerController.h
//  TestPSMove
//
//  Created by Chadwick Boulay on 2015-02-17.
//  Copyright (c) 2015 EpicGames. All rights reserved.
//
#pragma once

#include "GameFramework/PlayerController.h"
#include "PSMoveDelegate.h"
#include "PSMovePlayerController.generated.h"

UCLASS()
class APSMovePlayerController : public APlayerController, public PSMoveDelegate
{
    GENERATED_UCLASS_BODY()

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;
};
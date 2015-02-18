// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PSMoveActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPSMoveActorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UPSMoveActorComponent();
    // Caled when the component is destroyed.
    ~UPSMoveActorComponent();

    // Called when the game starts
    virtual void InitializeComponent() override;

    // Called every frame
    virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

    UFUNCTION(BlueprintCallable, Category = PSMove)
    void Get6DOFData(FVector pos, FRotator rot);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    int8 ControllerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    int8 CameraId;

};

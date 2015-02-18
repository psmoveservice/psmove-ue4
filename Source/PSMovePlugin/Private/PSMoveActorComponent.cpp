// Fill out your copyright notice in the Description page of Project Settings.

#include "PSMovePluginPrivatePCH.h"
#include "PSMoveActorComponent.h"


// Sets default values for this component's properties
UPSMoveActorComponent::UPSMoveActorComponent()
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("UPSMoveActorComponent::UPSMoveActorComponent"));
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    bWantsInitializeComponent = true;
    bAutoActivate = true;
    PrimaryComponentTick.bCanEverTick = true;

    ControllerId = 0;
    CameraId = 1;
    // ...
}

UPSMoveActorComponent::~UPSMoveActorComponent()
{
    UE_LOG(LogPSMovePlugin, Log, TEXT("UPSMoveActorComponent::~UPSMoveActorComponent"));
    //Do some data cleanup.
}

// Called when the game starts
void UPSMoveActorComponent::InitializeComponent()
{
    Super::InitializeComponent();
    UE_LOG(LogPSMovePlugin, Log, TEXT("UPSMoveActorComponent::InitializeComponent"));
    // ...
	
}

// Called every frame
void UPSMoveActorComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
    Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
    UE_LOG(LogPSMovePlugin, Log, TEXT("UPSMoveActorComponent::TickComponent"));
    // ...
}

void UPSMoveActorComponent::Get6DOFData(FVector pos, FRotator rot)
{
    
}
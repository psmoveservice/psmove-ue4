#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init) : UActorComponent(init)
{
    bWantsInitializeComponent = true;
    bAutoActivate = true;
    PrimaryComponentTick.bCanEverTick = true;
}

void UPSMoveComponent::OnRegister()
{
    Super::OnRegister();
    MoveSetup();
}

void UPSMoveComponent::OnUnregister()
{
    Super::OnUnregister();
    MoveUnset();
}

void UPSMoveComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    MoveTick(DeltaTime);
}
#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init) : UActorComponent(init)
{
    bWantsInitializeComponent = true;
    bAutoActivate = true;
    PrimaryComponentTick.bCanEverTick = true;
}

void UPSMoveComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    RefreshPQ(); // Updates values stored in member variables Position and Orientation.
}
#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "IPSMove.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init)
    : UActorComponent(init)
{
    bWantsInitializeComponent = true;
    bAutoActivate = true;
    PrimaryComponentTick.bCanEverTick = true;
}

void UPSMoveComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (IPSMove::IsAvailable())
    {
        IPSMove::Get().CopyPQ(Position, Orientation);
    }
    this->PositionAndOrientationUpdated(Position, Orientation);
}
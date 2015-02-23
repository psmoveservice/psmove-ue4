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

const FVector UPSMoveComponent::GetPosition() const
{
    if (IPSMove::IsAvailable())
    {
        return IPSMove::Get().GetPosition();
    }
    return FVector(0.0);
}

const FQuat UPSMoveComponent::GetOrientation() const
{
    if (IPSMove::IsAvailable())
    {
        return IPSMove::Get().GetOrientation();
    }
    return FQuat(0.0, 0.0, 0.0, 1.0);
}

const FRotator UPSMoveComponent::GetRotator() const
{
    FRotator rotator = FRotator(0.0);
    if (IPSMove::IsAvailable())
    {
        rotator = IPSMove::Get().GetOrientation().Rotator();
        rotator.Pitch *= -1;
        rotator.Yaw *= -1;
    }
    return rotator;
}
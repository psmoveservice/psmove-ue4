#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init)
: PSMoveID(0)
{
    bWantsInitializeComponent = true;
    PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UPSMoveComponent::InitializeComponent()
{
    Super::InitializeComponent();
    if (FPSMove::IsAvailable())
    {
        FPSMove::Get().InitWorker();
    }
}

// Called every frame
void UPSMoveComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
    Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

    UpdateData(DeltaTime);

}

void UPSMoveComponent::UpdateData( float DeltaSeconds )
{
    OnDataUpdatedImpl( PSMoveID, DeltaSeconds );  // TODO: Make first argument something useful.
    OnDataUpdated.Broadcast( PSMoveID, DeltaSeconds );
}

void UPSMoveComponent::OnDataUpdatedImpl(int32 MoveID, float DeltaSeconds)
{
    if (FPSMove::IsAvailable())
    {
        Position = FPSMove::Get().GetPosition(MoveID);
        Rotation = FPSMove::Get().GetOrientation(MoveID).Rotator();
        Rotation.Pitch *= -1;
        Rotation.Yaw *= -1;
    }
}
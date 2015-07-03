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
        FPSMove::Get().GetRawDataFramePtr(PSMoveID, DataFrame.RawDataPtr);
    }
}

// Called every frame
void UPSMoveComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
    Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

    if (FPSMove::IsAvailable())
    {
        // TODO: Check to see if it is necessary to update the DataFrame.RawDataPtr
        //FPSMove::Get().GetRawDataFramePtr(PSMoveID, DataFrame.RawDataPtr);
        
        OnDataUpdated.Broadcast(DataFrame.GetPosition(), DataFrame.GetRotation());
        OnTriangleButton.Broadcast(DataFrame.GetButtonTriangle());
        OnCircleButton.Broadcast(DataFrame.GetButtonCircle());
        OnCrossButton.Broadcast(DataFrame.GetButtonCross());
        OnSquareButton.Broadcast(DataFrame.GetButtonSquare());
        OnSelectButton.Broadcast(DataFrame.GetButtonSelect());
        OnStartButton.Broadcast(DataFrame.GetButtonStart());
        OnPSButton.Broadcast(DataFrame.GetButtonPS());
        OnMoveButton.Broadcast(DataFrame.GetButtonMove());
        OnTriggerButton.Broadcast(DataFrame.GetTriggerValue());
        
        DataFrame.SetRumbleRequest(RumbleRequest);
        DataFrame.SetResetPoseRequest(ResetPoseRequest);
    }
}
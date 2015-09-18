#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"
#include "Runtime/HeadMountedDisplay/Public/HeadMountedDisplay.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init) : Super(init)
{
    DataContextPtr= nullptr;
    bWantsInitializeComponent = true;
    PrimaryComponentTick.bCanEverTick = true;
	PlayerIndex = 0;
	Hand = EControllerHand::Right;
    StartingColourIndex = 0;
}

// Called when the game starts
void UPSMoveComponent::BeginPlay()
{
    Super::BeginPlay();

    if (FPSMove::IsAvailable())
    {
        // Bind the data context to the concurrent controller data in the worker thread
        if (FPSMove::Get().AcquirePSMove(this->PlayerIndex, this->Hand, &this->DataContextPtr))
        {
            if (bShowHMDFrustumDebug)
            {
                this->DataContextPtr->SetShowHMDFrustumDebug(true);
            }

            if (bShowTrackingDebug)
            {
                this->DataContextPtr->SetShowTrackingDebug(true);
            }
        }
        else
        {
            UE_LOG(LogPSMove, Error, TEXT("Failed to acquire PSMove controller %d/%d,"), this->PlayerIndex, (int32)this->Hand.GetValue());
        }
    }
}

void UPSMoveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (this->DataContextPtr != nullptr)
    {
        FPSMove::Get().ReleasePSMove(this->DataContextPtr);
        this->DataContextPtr= nullptr;
    }
}

void UPSMoveComponent::ResetYaw()
{
    if (DataContextPtr != nullptr)
    {
        DataContextPtr->Pose.SnapshotOrientationYaw();
    }
}

void UPSMoveComponent::ResetPosition()
{
    if (DataContextPtr != nullptr && (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D()))
    {
        DataContextPtr->Pose.SnapshotPosition();
    }
    else
    {
        DataContextPtr->Pose.ResetPositionSnapshot();
    }
}

void UPSMoveComponent::CycleColours()
{
    if (DataContextPtr != nullptr)
    {
        DataContextPtr->PostCycleColourRequest();
    }
}

bool UPSMoveComponent::GetIsTracking()
{
    bool isTracking = false;
    if (DataContextPtr != nullptr)
    {
        isTracking = DataContextPtr->GetIsTracking();
    }
    return isTracking;
}
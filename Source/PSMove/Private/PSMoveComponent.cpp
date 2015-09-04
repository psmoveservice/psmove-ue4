#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"
#include "Runtime/HeadMountedDisplay/Public/HeadMountedDisplay.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init) : 
    DataContextPtr(nullptr)
{
    bWantsInitializeComponent = true;
    PrimaryComponentTick.bCanEverTick = true;
	PlayerIndex = 0;
	Hand = EControllerHand::Right;
	ApplyTransformToActor = false;
}

// Called when the game starts
void UPSMoveComponent::BeginPlay()
{
    Super::BeginPlay();

    if (FPSMove::IsAvailable())
    {
        // Bind the data context to the concurrent controller data in the worker thread
        if (!FPSMove::Get().AcquirePSMove(this->PlayerIndex, this->Hand, &this->DataContextPtr))
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

// Called every frame
void UPSMoveComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
    Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// Optionally update the parent actor position
	if (ApplyTransformToActor)
	{
        if (this->DataContextPtr != nullptr)
        {
			FRotator Orientation= FRotator(this->DataContextPtr->Pose.PSMOri);
			FVector Position= this->DataContextPtr->Pose.PSMPos;

			GetOwner()->SetActorLocationAndRotation(Position, Orientation);
        }
    }
}

void UPSMoveComponent::ResetYaw()
{
    if (DataContextPtr != nullptr)
    {
        DataContextPtr->Pose.ResetYaw();
    }
}

void UPSMoveComponent::ResetPose()
{
    if (DataContextPtr != nullptr)
    {
        DataContextPtr->PostResetPoseRequest();
    }
}

void UPSMoveComponent::CycleColours()
{
    if (DataContextPtr != nullptr)
    {
        DataContextPtr->PostCycleColourRequest();
    }
}
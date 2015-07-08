#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"
#include "Runtime/HeadMountedDisplay/Public/HeadMountedDisplay.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init)
    : PSMoveID(0), UseHMDCorrection(true), ViewRotation(FRotator::ZeroRotator), ViewLocation(0.0)
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

        FVector BaseOffset(0.0);
        FQuat BaseOrientation = FQuat::Identity;
        FMatrix camMat = FMatrix::Identity;
        FVector HeadPosition(0.0);
        FQuat DeltaControlOrientation = FQuat::Identity;
        if (GEngine->HMDDevice.IsValid() && GEngine->HMDDevice->IsHMDEnabled())
        {
            BaseOffset = GEngine->HMDDevice->GetBaseOffset();
            BaseOrientation = GEngine->HMDDevice->GetBaseOrientation();

            // Get the camera pose and transform it back into Oculus space.
            FVector CamOrigin;
            FQuat CamOrientation;
            float CamHFOV;
            float CamVFOV;
            float CameraDistance;
            float CamNearPlane;
            float CamFarPlane;
            GEngine->HMDDevice->GetPositionalTrackingCameraProperties(CamOrigin, CamOrientation, CamHFOV, CamVFOV, CameraDistance, CamNearPlane, CamFarPlane);
            CamOrigin = BaseOrientation.RotateVector(CamOrigin);
            // TODO: CamOrigin /= CameraScale3D
            CamOrigin += BaseOffset;
            CamOrigin = FVector(CamOrigin.Y, CamOrigin.Z, -CamOrigin.X);
            CamOrientation = BaseOrientation * CamOrientation;
            CamOrientation.Normalize();
            camMat = FQuatRotationTranslationMatrix(CamOrientation, CamOrigin);

            // Get the HMD pose
            FQuat HeadOrient;
            GEngine->HMDDevice->GetCurrentOrientationAndPosition(HeadOrient, HeadPosition);
            DeltaControlOrientation = ViewRotation.Quaternion() * HeadOrient.Inverse();
        }

        // Get the raw PSMove position and quaternion
        FVector PSMPos = DataFrame.GetPosition();
        FQuat PSMOri = DataFrame.GetOrientation();

        // Transform it through the Oculus camera
        PSMPos = camMat.TransformPosition(PSMPos);

        // Convert to UE4 coordinate system.
        PSMPos = FVector(-PSMPos.Z, PSMPos.X, PSMPos.Y);
        PSMPos -= BaseOffset;
        // TODO: PSMPos *= CameraScale3D
        PSMPos = BaseOrientation.Inverse().RotateVector(PSMPos);
        PSMOri = BaseOrientation.Inverse() * PSMOri;
        PSMOri.Normalize();

        // Further transforms
        // See here: https://github.com/EpicGames/UnrealEngine/blob/master/Engine/Plugins/Runtime/GearVR/Source/GearVR/Private/HeadMountedDisplayCommon.cpp#L989
        PSMOri = DeltaControlOrientation * PSMOri;
        PSMPos = PSMPos - HeadPosition;
        PSMPos = DeltaControlOrientation.RotateVector(PSMPos);
        FMatrix m(FMatrix::Identity);
        m = PSMOri * m;
        //TODO: m *= FScaleMatrix(frame->CameraScale3D);
        m *= FTranslationMatrix(PSMPos);
        m *= FTranslationMatrix(ViewLocation); // to location of pawn
        PSMPos = m.TransformPosition(FVector(0.0));

        // Fire off the events
        OnDataUpdated.Broadcast(PSMPos, PSMOri.Rotator());
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
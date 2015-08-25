#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"
#include "Runtime/HeadMountedDisplay/Public/HeadMountedDisplay.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init) :
    PSMoveID(0),
    UseHMDCorrection(true),
    ZeroYaw(FQuat::Identity)
{
    bWantsInitializeComponent = true;
    PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UPSMoveComponent::BeginPlay()
{
    Super::BeginPlay();
    if (FPSMove::IsAvailable())
    {
        this->DataContext.Clear();

        // Bind the data context to the concurrent controller data in the worker thread
        if (!FPSMove::Get().AcquirePSMove(this->PSMoveID, &this->DataContext))
        {
            UE_LOG(LogPSMove, Error, TEXT("Failed to acquire PSMove controller %d"), this->PSMoveID);
        }
    }
}

void UPSMoveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (FPSMove::IsAvailable() && this->DataContext.PSMoveID != -1)
    {
        FPSMove::Get().ReleasePSMove(&this->DataContext);
    }
}

// Called every frame
void UPSMoveComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
    Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

    if (FPSMove::IsAvailable() && this->DataContext.PSMoveID != -1)
    {
        // Post component driven requests
        DataContext.SetRumbleRequest(this->RumbleRequest);
        DataContext.SetLedColourRequest(this->LedRequest);
        DataContext.SetResetPoseRequest(this->ResetPoseRequest);

        // Post the above data to the worker thread
        // and read the worker thread data the component cares about.
        DataContext.ComponentPostAndRead();

        // Get the raw PSMove position and quaternion
        FVector PSMPos = DataContext.GetPosition();
        FQuat PSMOri = DataContext.GetOrientation();
        bool PSMIsTracked = DataContext.GetIsTracking();

        /*
        There are several steps needed to go from the PSMove reference frame to the game world reference frame.
        These depend on whether or not an HMD is being used.
        */

        if (UseHMDCorrection && GEngine->HMDDevice.IsValid())
        {
            /*
            If an HMD is present and enabled, then assume that the PSMove coordinates
            are being returned in the the HMD_camera reference frame.
            The reason for using the HMD_camera and not the HMD_proper
            is that the physical relationship between the PSEye camera and the
            HMD_camera will change infrequently, but the relationship between the
            PSEye and the HMD_proper will change whenever the user recenters the pose.

            HMD_camera in HMD_CS -> HMD_native in HMD_CS -> HMD_native in UE4_CS -> HMD_UE4 in UE4_CS
            Where _camera is the camera reference frame, _native is the native reference frame (i.e., oculus API ref. frame)
            and _CS means 'coordinate system', with HMD_CS being RH and UE4_CS being LH
            */
            EHMDDeviceType::Type HMDDtype = GEngine->HMDDevice->GetHMDDeviceType(); // EHMDDeviceType::DT_OculusRift

            // Get the camera pose in HMD_UE4 in UE4_CS. This transforms from HMD_camera to HMD_native
            FVector CamOrigin;
            FQuat CamOrientation;
            float CamHFOV;
            float CamVFOV;
            float CameraDistance;
            float CamNearPlane;
            float CamFarPlane;
            GEngine->HMDDevice->GetPositionalTrackingCameraProperties(CamOrigin, CamOrientation, CamHFOV, CamVFOV, CameraDistance, CamNearPlane, CamFarPlane);

            FVector CameraScale3D(1.0); // TODO: Get this from the HMD, but as far as I can tell this
                                        // only changes if GetCurrentHMDPose is called with scale parameter

            // Get the HMD_UE4 origin in HMD_native, both in UE4_CS
            FVector HMDOrigin = GEngine->HMDDevice->GetBaseOffset(); // Custom addition to the engine by me.
            FQuat HMDZeroYaw = GEngine->HMDDevice->GetBaseOrientation();

            // Transform camera pose from HMD_UE4 space in UE4_CS to HMD_native in UE4_CS
            // Currently Oculus-specific.
            // TODO: CamOrigin -= frame->Settings->PositionOffset;  // Source says this is deprecated.
            CamOrientation = HMDZeroYaw * CamOrientation;
            CamOrientation.Normalize();
            CamOrigin = HMDZeroYaw.RotateVector(CamOrigin);
            CamOrigin /= CameraScale3D;
            CamOrigin += HMDOrigin;

            // Transform camera pose from HMD_native in UE4_CS to HMD_native in HMD_CS
            CamOrigin = FVector(CamOrigin.Y, CamOrigin.Z, -CamOrigin.X);  // Convert to native HMD coordinate system
            // TODO: units m->cm unnecessary because physical_transform was kept in cm.
            CamOrientation = FQuat(CamOrientation.Y, CamOrientation.Z, -CamOrientation.X, -CamOrientation.W);  // Convert to native HMD coordinate system

            // Transform the PSMove pose through the camera from HMD_camera in HMD_CS to HMD_native in HMD_CS
            PSMPos = CamOrientation.RotateVector(PSMPos);
            PSMPos += CamOrigin;
            PSMOri = CamOrientation * PSMOri;

            // Transform PSMove pose from HMD_native in HMD_CS to HMD_native in UE4_CS
            // Currently Oculus-specific.
            PSMOri = FQuat(PSMOri.Y, PSMOri.X, PSMOri.Z, -PSMOri.W);
            // TODO: units m->cm unnecessary because physical_transform was kept in cm.
            PSMPos = FVector(-PSMPos.Z, PSMPos.X, PSMPos.Y);

            // Transform PSMove pose from HMD_native in UE4_CS to HMD_UE4 in UE4_CS
            PSMPos -= HMDOrigin;
            PSMPos *= CameraScale3D;
            PSMPos = HMDZeroYaw.Inverse().RotateVector(PSMPos);
            PSMOri = HMDZeroYaw.Inverse() * PSMOri;
            PSMOri.Normalize();
            // TODO: PSMPos += frame->Settings->PositionOffset // Source says this is deprecated.
        }
        else
        {
            /*
            If we are not using an HMD then assume that the PSMove coordinates being returned are in their native space.
            */
            // Transform from PSEye coordinate system to UE4 coordinate system
            PSMOri = FQuat(PSMOri.Y, PSMOri.X, PSMOri.Z, -PSMOri.W);
            PSMPos = FVector(-PSMPos.Z, PSMPos.X, PSMPos.Y);
        }

        // Get the CameraManager
        UObject* WorldContextObject = this;
        APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, this->PlayerIndex);
        FTransform CameraManagerTransform = CameraManager->GetTransform();

        /*
        Now we must transform from HMD_UE4 space to world_UE4 space.
        This is done by transforming through the player camera which contains the player pose in the world.
        */

        FQuat DeltaControlOrientation = CameraManagerTransform.GetRotation();
        if (UseHMDCorrection && GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D())
        {
            // If using an HMD, it is necessary to undo the transformation done to the camera by the HMD.
            // See here: https://github.com/EpicGames/UnrealEngine/blob/master/Engine/Plugins/Runtime/GearVR/Source/GearVR/Private/HeadMountedDisplayCommon.cpp#L989
            
            // Get the HMD pose
            FVector HeadPosition;
            FQuat HeadOrient;
            GEngine->HMDDevice->GetCurrentOrientationAndPosition(HeadOrient, HeadPosition);

            // Untransform the camera through the HMD
            DeltaControlOrientation = DeltaControlOrientation * HeadOrient.Inverse();
            //PSMPos -= HeadPosition;  // Not useful unless we know the head has moved since the camera was last updated with the head pose
        }
        
        // Transform the PSMove through the camera
        PSMOri = DeltaControlOrientation * PSMOri;
        PSMPos = DeltaControlOrientation.RotateVector(PSMPos);

        PSMPos += CameraManagerTransform.GetLocation();

        LastPosition = PSMPos;
        LastOrientation = PSMOri;

        PSMOri = ZeroYaw * PSMOri;

        // Fire off the events
        OnDataUpdated.Broadcast(PSMPos, PSMOri.Rotator(), PSMIsTracked);
        OnTriangleButton.Broadcast(DataContext.GetButtonTriangle());
        OnCircleButton.Broadcast(DataContext.GetButtonCircle());
        OnCrossButton.Broadcast(DataContext.GetButtonCross());
        OnSquareButton.Broadcast(DataContext.GetButtonSquare());
        OnSelectButton.Broadcast(DataContext.GetButtonSelect());
        OnStartButton.Broadcast(DataContext.GetButtonStart());
        OnPSButton.Broadcast(DataContext.GetButtonPS());
        OnMoveButton.Broadcast(DataContext.GetButtonMove());
        OnTriggerButton.Broadcast(DataContext.GetTriggerValue());
    }
}

void UPSMoveComponent::ResetYaw()
{
    ZeroYaw = LastOrientation;
    ZeroYaw.X = 0;
    ZeroYaw.Y = 0;
    ZeroYaw.Normalize();
    ZeroYaw = ZeroYaw.Inverse();
}
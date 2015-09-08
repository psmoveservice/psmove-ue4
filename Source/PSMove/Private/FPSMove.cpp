//-- includes -----
#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "FPSMove.h"
#include "FPSMoveWorker.h"
#include "IMotionController.h"
#include "IInputDevice.h"
#include "IHeadMountedDisplay.h"

// Work in progress pose update method
#define USE_NEW_POSE_UPDATE

//-- pre-declarations -----
class FPSMoveInputManager;
class FPSMoveInternal;

//-- prototypes -----
static bool ComputeTrackingCameraFrustum(
    const int32 PlayerIndex,
    FTransform &TrackingSpaceToWorldSpace,
    float *TrackingCameraNearPlane = nullptr, 
    float *TrackingCameraFarPlane = nullptr, 
    float *TrackingCameraHHalfRadians = nullptr, 
    float *TrackingCameraVHalfRadians = nullptr);
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext);
static void DrawHMDTrackingFrustum();
static void DrawPSMoveTrackingDebug(FPSMoveDataContext *DataContext);
static void DrawDK2TrackingCameraFrustum(
    UWorld* World, 
    const FTransform& Transform, 
    const float NearLength, 
    const float FarLength, 
    const float HalfHorizontalRadians, 
    const float HalfVerticalRadians, 
    const FColor &Color);
static void DrawBasis(UWorld* World, const FTransform& Transform, const float Size);

//-- declarations -----
/**
* PSMove Input Device Manager - Handles IInputDevice and IMotionController Messaging
*/
class FPSMoveInputManager : public IInputDevice, public IMotionController
{
public:
	FPSMoveInputManager(FPSMoveInternal *InPlugin, const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
		: PSMovePlugin(InPlugin)
		, MessageHandler(InMessageHandler)
	{
		IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);

		//@todo:  fix this.  construction of the controller happens after InitializeMotionControllers(), so we manually insert into the array here.
		GEngine->MotionControllerDevices.AddUnique(this);
	}

	virtual ~FPSMoveInputManager()
	{
		GEngine->MotionControllerDevices.Remove(this);
	}

    void DrawDebug();

	/**
	* IInputDevice
	*/

	// Tick the interface 
	virtual void Tick(float DeltaTime) override;

	// Poll for controller state and send events if needed
	virtual void SendControllerEvents() override;

	// Set which MessageHandler will get the events from SendControllerEvents.
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	// Exec handler to allow console commands to be passed through for debugging 
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

	// IForceFeedbackSystem pass through function
	virtual void SetChannelValue(int32 PlayerIndex, FForceFeedbackChannelType ChannelType, float Value) override;

	// IForceFeedbackSystem pass through function
	virtual void SetChannelValues(int32 PlayerIndex, const FForceFeedbackValues &values) override;

	/**
	* IMotionController
	*/
	virtual bool GetControllerOrientationAndPosition(const int32 PlayerIndex, const EControllerHand DeviceHand, FRotator& OutOrientation, FVector& OutPosition) const;

protected:
	FGamepadKeyNames::Type GetButtonName(PSMove_Button Button, const EControllerHand Hand);

private:
	// Parent PSMovePlugin
	FPSMoveInternal *PSMovePlugin;

	// handler to send all messages to
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;
};

/**
* Private PSMovePlugin Module Implementation - Manages PSMoveController Context State
*/
class FPSMoveInternal : public FPSMove
{
public:
	/** Total number of controllers in a set */
	static const int32 ControllersPerPlayer = 2;

	/** Total number of motion controllers we'll support */
	static const int32 MaxControllers = FPSMoveWorker::k_max_controllers;

	/** The maximum number of Unreal controllers.  Each Unreal controller represents a set of motion controller devices */
	static const int32 MaxUnrealControllers = MaxControllers / ControllersPerPlayer;

	void StartupModule();
	void ShutdownModule();

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	virtual bool AcquirePSMove(int32 PlayerIndex, EControllerHand Hand, FPSMoveDataContext **OutDataContext) override;
	virtual void ReleasePSMove(FPSMoveDataContext *DataContext) override;

	const FPSMoveDataContext *GetDataContextByPlayerHandConst(const int32 PlayerIndex, const EControllerHand Hand) const;
	FPSMoveDataContext *GetDataContextByPlayerHand(const int32 PlayerIndex, const EControllerHand Hand);

	int32 GetDataContextCount();
	FPSMoveDataContext *GetDataContextByIndex(const int32 DataContextIndex);

	static void DataContextIndexToPlayerHand(const int DataContextIndex, int32 &OutPlayerIndex, EControllerHand &OutHand);
	static int32 PlayerHandToDataContextIndex(const int32 PlayerIndex, const EControllerHand Hand);

private:
	// Controller states
	FPSMoveDataContext ControllerDataContexts[MaxControllers];
};

IMPLEMENT_MODULE(FPSMoveInternal, PSMove)

//-- implementation -----

//-- FPSMovePlugin -----------------------

//-- FPSMovePCPlugin -----------------------
void FPSMoveInternal::StartupModule()
{
	FPSMove::StartupModule();

	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	UE_LOG(LogPSMove, Log, TEXT("Loading PSMove module..."));

	// Init the PSMoveWorker singleton.
	UE_LOG(LogPSMove, Log, TEXT("Initializing PSMoveWorker Thread..."));
	FPSMoveWorker::InitializeSingleton();
}

void FPSMoveInternal::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FPSMoveWorker::ShutdownSingleton();
	UE_LOG(LogPSMove, Log, TEXT("Shutdown PSMove module."));

	FPSMove::ShutdownModule();
}

TSharedPtr< class IInputDevice > FPSMoveInternal::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	return TSharedPtr< class IInputDevice >(new FPSMoveInputManager(this, InMessageHandler));
}

bool FPSMoveInternal::AcquirePSMove(int32 PlayerIndex, EControllerHand Hand, FPSMoveDataContext **OutDataContext)
{
	bool success = false;
	int32 DataContextIndex = PlayerHandToDataContextIndex(PlayerIndex, Hand);

	if (DataContextIndex != INDEX_NONE)
	{
		FPSMoveDataContext *DataContext = GetDataContextByIndex(DataContextIndex);
		FPSMoveWorker* WorkerSingleton = FPSMoveWorker::GetSingletonInstance();

		if (WorkerSingleton != NULL)
		{
			*OutDataContext = DataContext;
			success = WorkerSingleton->AcquirePSMove(PlayerIndex, DataContext);
		}
	}

	return success;
}

void FPSMoveInternal::ReleasePSMove(FPSMoveDataContext *DataContext)
{
	FPSMoveWorker* WorkerSingleton = FPSMoveWorker::GetSingletonInstance();
	if (WorkerSingleton != NULL)
	{
		WorkerSingleton->ReleasePSMove(DataContext);
	}
}

const FPSMoveDataContext *FPSMoveInternal::GetDataContextByPlayerHandConst(const int32 PlayerIndex, const EControllerHand Hand) const
{
	const FPSMoveDataContext *ControllerDataContex = nullptr;
	int32 DataContextIndex = PlayerHandToDataContextIndex(PlayerIndex, Hand);

	if (DataContextIndex != INDEX_NONE)
	{
		ControllerDataContex = &ControllerDataContexts[DataContextIndex];
	}

	return ControllerDataContex;
}

FPSMoveDataContext *FPSMoveInternal::GetDataContextByPlayerHand(const int32 PlayerIndex, const EControllerHand Hand)
{
	return const_cast<FPSMoveDataContext *>(GetDataContextByPlayerHandConst(PlayerIndex, Hand));
}

int32 FPSMoveInternal::GetDataContextCount()
{
	return MaxControllers;
}

FPSMoveDataContext *FPSMoveInternal::GetDataContextByIndex(const int32 DataContextIndex)
{
	return &ControllerDataContexts[DataContextIndex];
}

void FPSMoveInternal::DataContextIndexToPlayerHand(const int DataContextIndex, int32 &OutPlayerIndex, EControllerHand &OutHand)
{
	verify(DataContextIndex < MaxControllers);

	OutPlayerIndex = DataContextIndex / ControllersPerPlayer;
	OutHand = ((DataContextIndex % ControllersPerPlayer) == 0)
		? EControllerHand::Right // 0
		: EControllerHand::Left; // 1
}

int32 FPSMoveInternal::PlayerHandToDataContextIndex(const int32 PlayerIndex, const EControllerHand Hand)
{
	int32 HandIndex = (Hand == EControllerHand::Right) ? 0 : 1; // Right = 0, Left = 1
	int32 DataContextIndex = PlayerIndex*ControllersPerPlayer + HandIndex;

	return (DataContextIndex < MaxControllers) ? DataContextIndex : INDEX_NONE;
}

//-- FPSMoveInputManager -----------------------
void FPSMoveInputManager::Tick(float DeltaTime)
{
	for (int32 DataContextIndex = 0; DataContextIndex < PSMovePlugin->GetDataContextCount(); ++DataContextIndex)
	{
		FPSMoveDataContext *DataContext = PSMovePlugin->GetDataContextByIndex(DataContextIndex);

		// Poll the data from the worker thread
		DataContext->InputManagerPostAndRead();

        // Update the pose for the controller
        if (DataContext->GetIsEnabled())
        {
            int32 PlayerIndex;
            EControllerHand Hand;
            FPSMoveInternal::DataContextIndexToPlayerHand(DataContextIndex, PlayerIndex, Hand);

            DataContextPoseUpdate(PlayerIndex, DataContext);
        }
    }

    // Debug Rendering
    DrawDebug();
}

void FPSMoveInputManager::DrawDebug()
{    
    bool HasDrawnFrustum= false;

	for (int32 DataContextIndex = 0; DataContextIndex < PSMovePlugin->GetDataContextCount(); ++DataContextIndex)
	{
		FPSMoveDataContext *DataContext = PSMovePlugin->GetDataContextByIndex(DataContextIndex);

        if (DataContext->GetIsEnabled())
        {
            if (DataContext->ShowHMDFrustumDebug && !HasDrawnFrustum)
            {
                DrawHMDTrackingFrustum();
                HasDrawnFrustum= true;
            }

            if (DataContext->ShowTrackingDebug)
            {
                DrawPSMoveTrackingDebug(DataContext);
            }
        }
    }
}

void FPSMoveInputManager::SendControllerEvents()
{
	for (int32 DataContextIndex = 0; DataContextIndex < PSMovePlugin->GetDataContextCount(); ++DataContextIndex)
	{
		FPSMoveDataContext *DataContext = PSMovePlugin->GetDataContextByIndex(DataContextIndex);

		if (DataContext->GetIsEnabled() && !DataContext->GetInputManagerHasProcessedEvents())
		{
			int32 PlayerIndex;
			EControllerHand Hand;
			FPSMoveInternal::DataContextIndexToPlayerHand(DataContextIndex, PlayerIndex, Hand);

			if (DataContext->GetButtonTrianglePressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_TRIANGLE, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonTriangleReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_TRIANGLE, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonCirclePressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_CIRCLE, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonCircleReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_CIRCLE, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonCrossPressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_CROSS, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonCrossReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_CROSS, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonSquarePressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_SQUARE, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonSquareReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_SQUARE, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonSelectPressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_SELECT, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonSelectReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_SELECT, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonStartPressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_START, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonStartReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_START, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonPSPressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_PS, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonPSReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_PS, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonMovePressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_MOVE, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonMoveReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_MOVE, Hand), PlayerIndex, false);
			}

			if (DataContext->GetButtonTriggerPressed())
			{
				MessageHandler->OnControllerButtonPressed(GetButtonName(Btn_T, Hand), PlayerIndex, false);
			}
			else if (DataContext->GetButtonTriggerReleased())
			{
				MessageHandler->OnControllerButtonReleased(GetButtonName(Btn_T, Hand), PlayerIndex, false);
			}

			{
				uint8 PreviousTriggerValue = DataContext->GetPreviousTriggerValue();
				uint8 TriggerValue = DataContext->GetTriggerValue();

				if (TriggerValue != PreviousTriggerValue)
				{
					float AnalogTriggerValue = (float)DataContext->GetTriggerValue() / 255.f;

					MessageHandler->OnControllerAnalog(
						Hand == EControllerHand::Right
						? FGamepadKeyNames::MotionController_Right_TriggerAxis
						: FGamepadKeyNames::MotionController_Left_TriggerAxis,
						PlayerIndex, AnalogTriggerValue);
				}
			}

			// Flag that we have already processed the data so that we don't process stale data next frame
			DataContext->MarkInputManagerHasProcessedEvents();
		}
	}
}

void FPSMoveInputManager::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FPSMoveInputManager::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

void FPSMoveInputManager::SetChannelValue(int32 PlayerIndex, FForceFeedbackChannelType ChannelType, float Value)
{
	// Skip unless this is the left or right large channel, which we consider to be the only SteamVRController feedback channel
	if (ChannelType == FForceFeedbackChannelType::LEFT_LARGE || ChannelType == FForceFeedbackChannelType::RIGHT_LARGE)
	{
		const EControllerHand Hand =
			(ChannelType == FForceFeedbackChannelType::LEFT_LARGE)
			? EControllerHand::Left
			: EControllerHand::Right;
		FPSMoveDataContext *DataContext = PSMovePlugin->GetDataContextByPlayerHand(PlayerIndex, Hand);

		if (DataContext != nullptr)
		{
			uint8 RumbleValue = static_cast<uint8>(FMath::Clamp(Value*255.f, 0.f, 255.f));

			// This will get posted to the worker thread by ComponentPostAndRead() in Tick()
			DataContext->SetRumbleRequest(RumbleValue);
		}
	}
}

void FPSMoveInputManager::SetChannelValues(int32 PlayerIndex, const FForceFeedbackValues &values)
{
	SetChannelValue(PlayerIndex, FForceFeedbackChannelType::LEFT_LARGE, values.LeftLarge);
	SetChannelValue(PlayerIndex, FForceFeedbackChannelType::RIGHT_LARGE, values.RightLarge);
}

bool FPSMoveInputManager::GetControllerOrientationAndPosition(
    const int32 PlayerIndex, const EControllerHand DeviceHand, 
    FRotator& OutOrientation, FVector& OutPosition) const
{
	bool RetVal = false;
    static int32 DataContextIndex= FPSMoveInternal::PlayerHandToDataContextIndex(PlayerIndex, DeviceHand);
	const FPSMoveDataContext *ControllerDataContext = PSMovePlugin->GetDataContextByIndex(DataContextIndex);

	if (ControllerDataContext != nullptr && ControllerDataContext->GetIsEnabled())
	{
        const FPSMoveDataContext *DataContext= PSMovePlugin->GetDataContextByIndex(DataContextIndex);

        OutOrientation= FRotator(DataContext->Pose.WorldOrientation);
        OutPosition= DataContext->Pose.WorldPosition;

		RetVal = true;
	}

	return RetVal;
}

FGamepadKeyNames::Type FPSMoveInputManager::GetButtonName(PSMove_Button Button, const EControllerHand Hand)
{
	FGamepadKeyNames::Type Result = FGamepadKeyNames::Invalid;

	if (Hand == EControllerHand::Right)
	{
		switch (Button)
		{
		case Btn_TRIANGLE:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton1;
			break;
		case Btn_CIRCLE:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton2;
			break;
		case Btn_CROSS:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton3;
			break;
		case Btn_SQUARE:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton4;
			break;
		case Btn_SELECT:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton5;
			break;
		case Btn_START:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton6;
			break;
		case Btn_PS:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton7;
			break;
		case Btn_MOVE:
			Result = FGamepadKeyNames::MotionController_Right_FaceButton8;
			break;
		case Btn_T:
			Result = FGamepadKeyNames::MotionController_Right_Trigger;
			break;
		}
	}
	else
	{
		switch (Button)
		{
		case Btn_TRIANGLE:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton1;
			break;
		case Btn_CIRCLE:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton2;
			break;
		case Btn_CROSS:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton3;
			break;
		case Btn_SQUARE:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton4;
			break;
		case Btn_SELECT:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton5;
			break;
		case Btn_START:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton6;
			break;
		case Btn_PS:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton7;
			break;
		case Btn_MOVE:
			Result = FGamepadKeyNames::MotionController_Left_FaceButton8;
			break;
		case Btn_T:
			Result = FGamepadKeyNames::MotionController_Left_Trigger;
			break;
		}
	}

	return Result;
}

//-- Helper Functions -----------------------
#ifdef USE_NEW_POSE_UPDATE
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext)
{
    FTransform TrackingSpaceToWorldSpace;

    if (ComputeTrackingCameraFrustum(PlayerIndex, TrackingSpaceToWorldSpace))
    {
        ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);

        // The PSMove position is given in the space of the rift camera in centimeters
        float CentimetersToUnrealUnits= UHeadMountedDisplayFunctionLibrary::GetWorldToMetersScale(LocalPlayer) / 100.f;
        FVector PSMPosTrackingSpace= DataContext->GetPosition() * CentimetersToUnrealUnits;
        FVector PSMPosWorldSpace= TrackingSpaceToWorldSpace.TransformPosition(PSMPosTrackingSpace);

        // Transform PSMove pose from HMD_native in HMD_CS to HMD_native in UE4_CS
        // Currently Oculus-specific.
        FQuat PSMOriNative = DataContext->GetOrientation();
        FQuat PSMOriWorld = FQuat(PSMOriNative.Y, PSMOriNative.X, PSMOriNative.Z, -PSMOriNative.W);        

        // Save the resulting pose
        FPSMovePose *Pose = &DataContext->Pose;

        Pose->WorldPosition = PSMPosWorldSpace;

        Pose->UncorrectedWorldOrientation = PSMOriWorld;
        Pose->WorldOrientation = Pose->ZeroYaw * PSMOriWorld;
    }
}

#else
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext)
{
    // Get the PSMove position and quaternion in the space of the DK2 tracking camera in cm
    FVector PSMPos = DataContext->GetPosition();
    FQuat PSMOri = DataContext->GetOrientation();

    /*
    There are several steps needed to go from the PSMove reference frame to the game world reference frame.
    These depend on whether or not an HMD is being used.
    */

    if (GEngine->HMDDevice.IsValid())
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
    ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);
    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(LocalPlayer, PlayerIndex);
    FTransform CameraManagerTransform = CameraManager->GetTransform();

    /*
    Now we must transform from HMD_UE4 space to world_UE4 space.
    This is done by transforming through the player camera which contains the player pose in the world.
    */

    FQuat DeltaControlOrientation = CameraManagerTransform.GetRotation();
    if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D())
    {
        // If using an HMD, it is necessary to undo the transformation done to the camera by the HMD.
        // See here: https://github.com/EpicGames/UnrealEngine/blob/master/Engine/Plugins/Runtime/GearVR/Source/GearVR/Private/HeadMountedDisplayCommon.cpp#L989
            
        // Get the HMD pose in UE4 CS (in UU)
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

    // Save the resulting pose
    {
        FPSMovePose *Pose = &DataContext->Pose;

        Pose->WorldPosition = PSMPos;

        Pose->UncorrectedWorldOrientation = PSMOri;
        Pose->WorldOrientation = Pose->ZeroYaw * PSMOri;
    }
}
#endif

static FTransform RiftToUnrealCoordinateSystemTransform()
{
    // Rift Coordinate System -> Unreal Coordinate system ==> (x, y, z) -> (-z, x, y)
    // This tranform is equivalent to Scale_z(-1)*Rot_y(90)*Rot_x(90)
    return FTransform(FMatrix(FVector(0, 1, 0), FVector(0, 0, 1), FVector(-1, 0, 0), FVector::ZeroVector));
}

static bool ComputeTrackingCameraFrustum(
    const int32 PlayerIndex,
    FTransform &TrackingSpaceToWorldSpace, // In Unreal Units
    float *OutTrackingCameraNearPlane, // In Unreal Units
    float *OutTrackingCameraFarPlane, // In Unreal Units
    float *OutTrackingCameraHHalfRadians, 
    float *OutTrackingCameraVHalfRadians)
{
    bool success= false;

    // Get the CameraManager for the primary player
    ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);
    APlayerCameraManager* GameCameraManager = UGameplayStatics::GetPlayerCameraManager(LocalPlayer, PlayerIndex);
    

    if (GameCameraManager != nullptr)
    {
        FTransform RiftToUnrealCS= RiftToUnrealCoordinateSystemTransform();

        // Get the world game camera transform for the player
        FQuat GameCameraOrientation= GameCameraManager->GetCameraRotation().Quaternion();
        FVector GameCameraLocation= GameCameraManager->GetCameraLocation();

        // Get the HMD pose in UE4 Coordinate System (in Unreal Units)
        FVector HMDPosition;
        FQuat HMDOrientation;
        GEngine->HMDDevice->GetCurrentOrientationAndPosition(HMDOrientation, HMDPosition);

        // Get the camera pose in HMD_UE4 in UE4_CS. This transforms from HMD_camera to HMD_native
        FVector TrackingCameraOrigin;
        FQuat TrackingCameraOrientation;
        float TrackingCameraHFOVDegrees;
        float TrackingCameraVFOVDegrees;
        float TrackingCameraDefaultDistance;
        float TrackingCameraNearPlane;
        float TrackingCameraFarPlane;
        GEngine->HMDDevice->GetPositionalTrackingCameraProperties(
            TrackingCameraOrigin, TrackingCameraOrientation, 
            TrackingCameraHFOVDegrees, TrackingCameraVFOVDegrees, 
            TrackingCameraDefaultDistance, 
            TrackingCameraNearPlane, TrackingCameraFarPlane);   

        // HMDToGameCameraRotation = Undo HMD orientation THEN apply game camera orientation
        FQuat HMDToGameCameraRotation= GameCameraOrientation * HMDOrientation.Inverse();
        FQuat TrackingCameraToGameRotation= HMDToGameCameraRotation * TrackingCameraOrientation;

        // Compute the tracking camera location in world space
        FVector TrackingCameraWorldSpaceOrigin=
            HMDToGameCameraRotation.RotateVector(TrackingCameraOrigin) + GameCameraLocation;

        // Optional tracking frustum properties
        if (OutTrackingCameraNearPlane != nullptr)
        {
            *OutTrackingCameraNearPlane= TrackingCameraNearPlane;
        }

        if (OutTrackingCameraFarPlane != nullptr)
        {
            *OutTrackingCameraFarPlane= TrackingCameraFarPlane;
        }

        if (OutTrackingCameraHHalfRadians != nullptr)
        {
            *OutTrackingCameraHHalfRadians= FMath::DegreesToRadians(TrackingCameraHFOVDegrees/2.f);
        }

        if (OutTrackingCameraVHalfRadians != nullptr)
        {
            *OutTrackingCameraVHalfRadians= FMath::DegreesToRadians(TrackingCameraVFOVDegrees/2.f);
        }

        // Compute the Transform to go from Rift Tracking Space (in unreal units) to World Tracking Space (in unreal units)
        TrackingSpaceToWorldSpace= 
            RiftToUnrealCS * FTransform(TrackingCameraToGameRotation, TrackingCameraWorldSpaceOrigin);

        success= true;
    }

    return success;
}

static void DrawHMDTrackingFrustum()
{
    const int32 PlayerIndex= 0;
    FTransform TrackingSpaceToWorldSpace;
    float TrackingCameraNearPlane; 
    float TrackingCameraFarPlane; 
    float TrackingCameraHHalfRadians;
    float TrackingCameraVHalfRadians;

    if (ComputeTrackingCameraFrustum(
            PlayerIndex,
            TrackingSpaceToWorldSpace,
            &TrackingCameraNearPlane, 
            &TrackingCameraFarPlane, 
            &TrackingCameraHHalfRadians, 
            &TrackingCameraVHalfRadians))
    {
        ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);

        DrawDK2TrackingCameraFrustum(
            LocalPlayer->GetWorld(), 
            TrackingSpaceToWorldSpace, 
            TrackingCameraNearPlane, 
            TrackingCameraFarPlane, 
            TrackingCameraHHalfRadians, 
            TrackingCameraVHalfRadians, 
            FColor::Yellow);
        DrawBasis(LocalPlayer->GetWorld(), TrackingSpaceToWorldSpace, 50.f);
    }
}

static void DrawPSMoveTrackingDebug(FPSMoveDataContext *DataContext)
{
    const int32 PlayerIndex= 0;
    FTransform TrackingSpaceToWorldSpace;

    if (ComputeTrackingCameraFrustum(PlayerIndex, TrackingSpaceToWorldSpace))
    {
        ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);

        // The PSMove position is given in the space of the rift camera in centimeters
        float CentimetersToUnrealUnits= UHeadMountedDisplayFunctionLibrary::GetWorldToMetersScale(LocalPlayer) / 100.f;
        FVector PSMoveInRiftTrackingSpace= DataContext->GetPosition() * CentimetersToUnrealUnits;
        FVector PSMoveInWorldSpace= TrackingSpaceToWorldSpace.TransformPosition(PSMoveInRiftTrackingSpace);

        DrawDebugSphere(LocalPlayer->GetWorld(), PSMoveInWorldSpace, 3.f, 15, FColor::White);
    }
}

static void DrawBasis(
    UWorld* World, 
    const FTransform& Transform,
    const float Size)
{
    FVector Origin= Transform.GetLocation();
    FVector XEndPoint= Transform.TransformPosition(FVector(Size, 0, 0));
    FVector YEndPoint= Transform.TransformPosition(FVector(0, Size, 0));
    FVector ZEndPoint= Transform.TransformPosition(FVector(0, 0, Size));

    DrawDebugLine(World, Origin, XEndPoint, FColor::Red);
    DrawDebugLine(World, Origin, YEndPoint, FColor::Green);
    DrawDebugLine(World, Origin, ZEndPoint, FColor::Blue);
}

static void DrawDK2TrackingCameraFrustum(
    UWorld* World, 
    const FTransform& TrackingToWorldTransform, 
    const float NearLength, 
    const float FarLength, 
    const float HalfHorizontalRadians, 
    const float HalfVerticalRadians, 
    const FColor &Color)
{
    const float HorizontalRatio= FMath::Tan(HalfHorizontalRadians);
    const float VerticalRatio= FMath::Tan(HalfVerticalRadians);

	const float HalfNearWidth = NearLength * HorizontalRatio;
	const float HalfNearHeight = NearLength * VerticalRatio;

	const float HalfFarWidth = FarLength * HorizontalRatio;
	const float HalfFarHeight = FarLength * VerticalRatio;

	const FVector Origin = TrackingToWorldTransform.GetLocation();

	const FVector NearV0 = TrackingToWorldTransform.TransformPosition(FVector(HalfNearWidth, HalfNearHeight, NearLength));
	const FVector NearV1 = TrackingToWorldTransform.TransformPosition(FVector(-HalfNearWidth, HalfNearHeight, NearLength));
	const FVector NearV2 = TrackingToWorldTransform.TransformPosition(FVector(-HalfNearWidth, -HalfNearHeight, NearLength));
	const FVector NearV3 = TrackingToWorldTransform.TransformPosition(FVector(HalfNearWidth, -HalfNearHeight, NearLength));

	const FVector FarV0 = TrackingToWorldTransform.TransformPosition(FVector(HalfFarWidth, HalfFarHeight, FarLength));
	const FVector FarV1 = TrackingToWorldTransform.TransformPosition(FVector(-HalfFarWidth, HalfFarHeight, FarLength));
	const FVector FarV2 = TrackingToWorldTransform.TransformPosition(FVector(-HalfFarWidth, -HalfFarHeight, FarLength));
	const FVector FarV3 = TrackingToWorldTransform.TransformPosition(FVector(HalfFarWidth, -HalfFarHeight, FarLength));

	DrawDebugLine(World, Origin, FarV0, Color);
	DrawDebugLine(World, Origin, FarV1, Color);
	DrawDebugLine(World, Origin, FarV2, Color);
	DrawDebugLine(World, Origin, FarV3, Color);

	DrawDebugLine(World, NearV0, NearV1, Color);
	DrawDebugLine(World, NearV1, NearV2, Color);
	DrawDebugLine(World, NearV2, NearV3, Color);
	DrawDebugLine(World, NearV3, NearV0, Color);

	DrawDebugLine(World, FarV0, FarV1, Color);
	DrawDebugLine(World, FarV1, FarV2, Color);
	DrawDebugLine(World, FarV2, FarV3, Color);
	DrawDebugLine(World, FarV3, FarV0, Color);
}

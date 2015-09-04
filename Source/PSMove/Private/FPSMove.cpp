//-- includes -----
#include "PSMovePrivatePCH.h"  // Also includes psmove api lib headers.

#include <assert.h>
#include "FPSMove.h"
#include "FPSMoveWorker.h"
#include "IMotionController.h"
#include "IInputDevice.h"
#include "IHeadMountedDisplay.h"

//-- pre-declarations -----
class FPSMoveInputManager;
class FPSMoveInternal;

//-- prototypes -----
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext);

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
    FPSMovePose *GetDataContextPoseByIndex(const int32 DataContextIndex);

	static void DataContextIndexToPlayerHand(const int DataContextIndex, int32 &OutPlayerIndex, EControllerHand &OutHand);
	static int32 PlayerHandToDataContextIndex(const int32 PlayerIndex, const EControllerHand Hand);

private:
	// Controller states
	FPSMoveDataContext ControllerDataContexts[MaxControllers];
    FPSMovePose ControllerPoses[MaxControllers];
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

FPSMovePose *FPSMoveInternal::GetDataContextPoseByIndex(const int32 DataContextIndex)
{
	return &ControllerPoses[DataContextIndex];
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
            FPSMovePose *Pose= PSMovePlugin->GetDataContextPoseByIndex(DataContextIndex);

            int32 PlayerIndex;
            EControllerHand Hand;
            FPSMoveInternal::DataContextIndexToPlayerHand(DataContextIndex, PlayerIndex, Hand);

            DataContextPoseUpdate(PlayerIndex, DataContext);
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
        const FPSMovePose *ControllerPose= PSMovePlugin->GetDataContextPoseByIndex(DataContextIndex);

        OutOrientation= FRotator(ControllerPose->PSMOri);
        OutPosition= ControllerPose->PSMPos;

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
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext)
{
    FPSMovePose *Pose = &DataContext->Pose;

    // Get the raw PSMove position and quaternion
    Pose->PSMPos = DataContext->GetPosition();
    Pose->PSMOri = DataContext->GetOrientation();

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
        Pose->PSMPos = CamOrientation.RotateVector(Pose->PSMPos);
        Pose->PSMPos += CamOrigin;
        Pose->PSMOri = CamOrientation * Pose->PSMOri;

        // Transform PSMove pose from HMD_native in HMD_CS to HMD_native in UE4_CS
        // Currently Oculus-specific.
        Pose->PSMOri = FQuat(Pose->PSMOri.Y, Pose->PSMOri.X, Pose->PSMOri.Z, -Pose->PSMOri.W);
        // TODO: units m->cm unnecessary because physical_transform was kept in cm.
        Pose->PSMPos = FVector(-Pose->PSMPos.Z, Pose->PSMPos.X, Pose->PSMPos.Y);

        // Transform PSMove pose from HMD_native in UE4_CS to HMD_UE4 in UE4_CS
        Pose->PSMPos -= HMDOrigin;
        Pose->PSMPos *= CameraScale3D;
        Pose->PSMPos = HMDZeroYaw.Inverse().RotateVector(Pose->PSMPos);
        Pose->PSMOri = HMDZeroYaw.Inverse() * Pose->PSMOri;
        Pose->PSMOri.Normalize();
        // TODO: PSMPos += frame->Settings->PositionOffset // Source says this is deprecated.
    }
    else
    {
        /*
        If we are not using an HMD then assume that the PSMove coordinates being returned are in their native space.
        */
        // Transform from PSEye coordinate system to UE4 coordinate system
        Pose->PSMOri = FQuat(Pose->PSMOri.Y, Pose->PSMOri.X, Pose->PSMOri.Z, -Pose->PSMOri.W);
        Pose->PSMPos = FVector(-Pose->PSMPos.Z, Pose->PSMPos.X, Pose->PSMPos.Y);
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
            
        // Get the HMD pose
        FVector HeadPosition;
        FQuat HeadOrient;
        GEngine->HMDDevice->GetCurrentOrientationAndPosition(HeadOrient, HeadPosition);

        // Untransform the camera through the HMD
        DeltaControlOrientation = DeltaControlOrientation * HeadOrient.Inverse();
        //PSMPos -= HeadPosition;  // Not useful unless we know the head has moved since the camera was last updated with the head pose
    }
        
    // Transform the PSMove through the camera
    Pose->PSMOri = DeltaControlOrientation * Pose->PSMOri;
    Pose->PSMPos = DeltaControlOrientation.RotateVector(Pose->PSMPos);

    Pose->PSMPos += CameraManagerTransform.GetLocation();

    Pose->LastPosition = Pose->PSMPos;
    Pose->LastOrientation = Pose->PSMOri;

    Pose->PSMOri = Pose->ZeroYaw * Pose->PSMOri;
}

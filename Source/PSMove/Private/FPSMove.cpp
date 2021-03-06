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

//-- constants -----
static float k_default_tracking_hfov_degrees = 74.f; // degrees
static float k_default_tracking_vfov_degrees = 54.f;  // degrees
static float k_default_tracking_distance = 1.5f; // meters
static float k_default_tracking_near_plane_distance = 0.4f; // meters
static float k_default_tracking_far_plane_distance = 2.5f; // meters

//-- prototypes -----
//-- Tracking Transform Helpers -----
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext);
static FQuat PSMoveQuatToUnrealQuat(const FQuat &PSMoveQuat);
static FTransform RHSCMToUnrealUUTransform(ULocalPlayer* LocalPlayer);
static bool ComputeTrackingToWorldTransforms(
    const int32 PlayerIndex, 
    FTransform &TrackingSpaceToWorldSpacePosition,
    FQuat &OrientationTransform);
static bool ComputeTrackingToWorldTransformsAndFrustum(
    const int32 PlayerIndex,
    FTransform &TrackingSpaceToWorldSpacePosition,
    FQuat &OrientationTransform,
    float &TrackingCameraNearPlane, 
    float &TrackingCameraFarPlane, 
    float &TrackingCameraHHalfRadians, 
    float &TrackingCameraVHalfRadians);

//-- Debug Rendering -----
static void DrawDebugTrackingCameraFrustum();
static void DrawDebugPSMoveWorldPosition(FPSMoveDataContext *DataContext);
static void DrawDebugRHSTrackingCameraFrustum(
    UWorld* World, 
    const FTransform& Transform, 
    const float NearLength, 
    const float FarLength, 
    const float HalfHorizontalRadians, 
    const float HalfVerticalRadians, 
    const FColor &Color);
static void DrawDebugBasis(UWorld* World, const FTransform& Transform, const float Size);

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
            //i.e. my_PSMoveComponent->DataContextPtr = &(my_FPSMoveSingleton.ControllerDataContexts[DataContextIndex])
            *OutDataContext = DataContext;

			success = WorkerSingleton->AcquirePSMove(DataContextIndex, DataContext);
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
                DrawDebugTrackingCameraFrustum();
                HasDrawnFrustum= true;
            }

            if (DataContext->ShowTrackingDebug)
            {
                DrawDebugPSMoveWorldPosition(DataContext);
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
    int32 DataContextIndex= FPSMoveInternal::PlayerHandToDataContextIndex(PlayerIndex, DeviceHand);
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

//-- Tracking Transform Helpers -----
static void DataContextPoseUpdate(const int32 PlayerIndex, FPSMoveDataContext *DataContext)
{     
    FTransform TrackingSpaceToWorldSpacePosition;
    FQuat OrientationTransform;

    if (ComputeTrackingToWorldTransforms(
            PlayerIndex,
            TrackingSpaceToWorldSpacePosition,
            OrientationTransform))
    {
        // The PSMove position is given in the space of the rift camera in centimeters
        FVector PSMPosTrackingSpace = DataContext->GetPosition();
        // Transform to world space
        FVector PSMPosWorldSpace= TrackingSpaceToWorldSpacePosition.TransformPosition(PSMPosTrackingSpace);

        // The PSMove orientation is given in its native coordinate system
        FQuat PSMOriNative = DataContext->GetOrientation();
        // NOTE: Intentionally backwards multiplication for quaternions
        // Apply controller orientation first, then apply orientation transform
        FQuat PSMOriWorld = OrientationTransform * PSMoveQuatToUnrealQuat(PSMOriNative);

        // Save the resulting pose, updating for internal offset/zeroyaw
        FPSMovePose *Pose = &DataContext->Pose;
        Pose->UncorrectedWorldPosition = PSMPosWorldSpace;
        Pose->WorldPosition = PSMPosWorldSpace - Pose->ZeroPosition;
        Pose->UncorrectedWorldOrientation = PSMOriWorld;
        Pose->WorldOrientation = Pose->ZeroYaw * PSMOriWorld;
    }
}

static FQuat PSMoveQuatToUnrealQuat(const FQuat &PSMoveQuat)
{
    return FQuat(PSMoveQuat.Y, PSMoveQuat.X, PSMoveQuat.Z, -PSMoveQuat.W);
}

static FTransform RHSCMToUnrealUUTransform(ULocalPlayer* LocalPlayer)
{
    // Convert from Right Handed (e.g., OpenGL, Oculus Rift native) to Unreal Left Handed coordinate system.
    // Also convert units from centimeters to unreal units (typically 1 cm == 1 UU, but player scale may change).

    // Conversion from centimeters to unreal units
    float CMToUU = UHeadMountedDisplayFunctionLibrary::GetWorldToMetersScale(LocalPlayer) / 100.f;

    // Rift Coordinate System -> Unreal Coordinate system ==> (x, y, z) -> (-z, x, y)
    // This tranform is equivalent to Scale_z(-1)*Rot_y(90)*Rot_x(90)
    return FTransform(FMatrix(FVector(0, CMToUU, 0), FVector(0, 0, CMToUU), FVector(-CMToUU, 0, 0), FVector::ZeroVector));
}

static bool ComputeTrackingToWorldTransforms(
    const int32 PlayerIndex,
    FTransform &TrackingSpaceToWorldSpacePosition,
    FQuat &OrientationTransform)
{
    float TrackingCameraNearPlane;
    float TrackingCameraFarPlane;
    float TrackingCameraHHalfRadians;
    float TrackingCameraVHalfRadians;

    return 
    ComputeTrackingToWorldTransformsAndFrustum(
        PlayerIndex,
        TrackingSpaceToWorldSpacePosition, // In Unreal Units
        OrientationTransform, // In Unreal Coordinate System
        TrackingCameraNearPlane, // In Unreal Units
        TrackingCameraFarPlane, // In Unreal Units
        TrackingCameraHHalfRadians, 
        TrackingCameraVHalfRadians);
}

static bool ComputeTrackingToWorldTransformsAndFrustum(
    const int32 PlayerIndex,
    FTransform &TrackingSpaceToWorldSpacePosition, // In Unreal Units
    FQuat &OrientationTransform, // In Unreal Coordinate System
    float &TrackingCameraNearPlane, // In Unreal Units
    float &TrackingCameraFarPlane, // In Unreal Units
    float &TrackingCameraHHalfRadians, 
    float &TrackingCameraVHalfRadians)
{
    bool success = false;

    // Get the CameraManager for the primary player
    ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);
    APlayerCameraManager* GameCameraManager = UGameplayStatics::GetPlayerCameraManager(LocalPlayer, PlayerIndex);
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(LocalPlayer, PlayerIndex);

    if (GameCameraManager != nullptr)
    {
        // Get the world game camera transform for the player
        FQuat GameCameraOrientation = GameCameraManager->GetCameraRotation().Quaternion();
        FVector GameCameraLocation = GameCameraManager->GetCameraLocation();

        // Get the orientation of the pawn controlling the player
        FRotator PlayerControllerRotation= PlayerController->GetControlRotation();

        if (GEngine->HMDDevice.IsValid() && GEngine->IsStereoscopic3D())
        {
            FVector HMDPosition;
            FQuat HMDOrientation;
            FVector TrackingCameraOrigin;
            FQuat TrackingCameraOrientation;
            float TrackingCameraDefaultDistance;
            float TrackingCameraHFOVDegrees;
            float TrackingCameraVFOVDegrees;

            // Get the HMD pose in player reference frame, UE4 CS (LHS), Unreal Units
            GEngine->HMDDevice->GetCurrentOrientationAndPosition(HMDOrientation, HMDPosition);

            // Get the camera pose in player reference frame, UE4 CS (LHS), Unreal Units
            GEngine->HMDDevice->GetPositionalTrackingCameraProperties(
                TrackingCameraOrigin, TrackingCameraOrientation,
                TrackingCameraHFOVDegrees, TrackingCameraVFOVDegrees,
                TrackingCameraDefaultDistance,
                TrackingCameraNearPlane, TrackingCameraFarPlane);

            TrackingCameraHHalfRadians= FMath::DegreesToRadians(TrackingCameraHFOVDegrees/2.f);
            TrackingCameraVHalfRadians= FMath::DegreesToRadians(TrackingCameraVFOVDegrees/2.f);

            // HMDToGameCameraRotation = Undo HMD orientation THEN apply game camera orientation
            FQuat HMDToGameCameraRotation = GameCameraOrientation * HMDOrientation.Inverse();
            FQuat TrackingCameraToGameRotation = HMDToGameCameraRotation * TrackingCameraOrientation;

            // Compute the tracking camera location in world space
            FVector TrackingCameraWorldSpaceOrigin =
                HMDToGameCameraRotation.RotateVector(TrackingCameraOrigin) + GameCameraLocation;

            // Compute the Transform to go from Rift Tracking Space (in unreal units) to World Tracking Space (in unreal units)
            TrackingSpaceToWorldSpacePosition = 
                RHSCMToUnrealUUTransform(LocalPlayer) *
                FTransform(TrackingCameraToGameRotation, TrackingCameraWorldSpaceOrigin);

            // Transform the orientation of the controller from world space to pawn space
            OrientationTransform= HMDToGameCameraRotation;
        }
        else 
        {
            // All default camera constants are in meters
            const float MetersToUU = UHeadMountedDisplayFunctionLibrary::GetWorldToMetersScale(LocalPlayer);

            // Pretend that the tracking camera is directly in front of the game camera
            const float FakeTrackingCameraOffset= k_default_tracking_distance * MetersToUU;
            FVector FakeTrackingCameraWorldSpaceOrigin = 
                GameCameraLocation + GameCameraOrientation.GetAxisX()*FakeTrackingCameraOffset;

            // Get tracking frustum properties from defaults
            TrackingCameraHHalfRadians= FMath::DegreesToRadians(k_default_tracking_hfov_degrees/2.f);
            TrackingCameraVHalfRadians= FMath::DegreesToRadians(k_default_tracking_vfov_degrees/2.f);
            TrackingCameraNearPlane= k_default_tracking_near_plane_distance * MetersToUU;
            TrackingCameraFarPlane= k_default_tracking_far_plane_distance * MetersToUU;

            // Compute the Transform to go from Rift Tracking Space (in unreal units) to World Tracking Space (in unreal units)
            TrackingSpaceToWorldSpacePosition = 
                // Convert from Right Handed (e.g., OpenGL, Oculus Rift native) to Unreal Left Handed coordinate system.
                // Also scale cm to unreal units
                RHSCMToUnrealUUTransform(LocalPlayer) *
                // Put in the orientation of the game camera
                FTransform(GameCameraOrientation, FakeTrackingCameraWorldSpaceOrigin);

            // Transform the orientation of the controller from world space to pawn space
            OrientationTransform= PlayerControllerRotation.Quaternion();
        }

        success = true;
    }

    return success;
}

//-- Debug Rendering -----
static void DrawDebugTrackingCameraFrustum()
{
    const int32 PlayerIndex= 0;
    FTransform TrackingSpaceToWorldSpacePosition;
    FQuat OrientationTransform;
    float TrackingCameraNearPlane; 
    float TrackingCameraFarPlane; 
    float TrackingCameraHHalfRadians;
    float TrackingCameraVHalfRadians;

    if (ComputeTrackingToWorldTransformsAndFrustum(
            PlayerIndex,
            TrackingSpaceToWorldSpacePosition,
            OrientationTransform,
            TrackingCameraNearPlane, 
            TrackingCameraFarPlane, 
            TrackingCameraHHalfRadians, 
            TrackingCameraVHalfRadians))
    {
        ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);

        DrawDebugRHSTrackingCameraFrustum(
            LocalPlayer->GetWorld(), 
            TrackingSpaceToWorldSpacePosition, 
            TrackingCameraNearPlane, 
            TrackingCameraFarPlane, 
            TrackingCameraHHalfRadians, 
            TrackingCameraVHalfRadians, 
            FColor::Yellow);
        DrawDebugBasis(LocalPlayer->GetWorld(), TrackingSpaceToWorldSpacePosition, 50.f); // 50 cm
    }
}

static void DrawDebugPSMoveWorldPosition(FPSMoveDataContext *DataContext)
{
    const int32 PlayerIndex= 0;
    FTransform TrackingSpaceToWorldSpacePosition;
    FQuat OrientationTransform;

    if (ComputeTrackingToWorldTransforms(
            PlayerIndex, 
            TrackingSpaceToWorldSpacePosition,
            OrientationTransform))
    {
        ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(PlayerIndex);
        FVector PSMovePosInWorldSpace= TrackingSpaceToWorldSpacePosition.TransformPosition(DataContext->GetPosition());
        
        // The PSMove orientation is given in its native coordinate system
        FQuat PSMOriNative = DataContext->GetOrientation();
        // NOTE: Intentially backwards multiplication for quatertions
        // Apply controller orientation first, then apply orientation transform
        FQuat PSMOriWorld = OrientationTransform * PSMoveQuatToUnrealQuat(PSMOriNative);

        DrawDebugSphere(LocalPlayer->GetWorld(), PSMovePosInWorldSpace, 3.f, 15, FColor::White);
        DrawDebugBasis(LocalPlayer->GetWorld(), FTransform(PSMOriWorld, PSMovePosInWorldSpace), 10.f); // 10 cm
    }
}

static void DrawDebugBasis(
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

// Draw Right Handed (e.g., OpenGL, Oculus Rift native) frustum oriented down the Z-Axis in given tracking space
static void DrawDebugRHSTrackingCameraFrustum(
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

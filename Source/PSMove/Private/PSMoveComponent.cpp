#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init)
: PSMoveID(0), RumbleRequest(0)
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

    if (FPSMove::IsAvailable())
    {
        // Get position and orientation info from module instance.
        Position = FPSMove::Get().GetPosition(PSMoveID);
        Rotation = FPSMove::Get().GetOrientation(PSMoveID).Rotator();
        Rotation.Pitch *= -1;
        Rotation.Yaw *= -1;
        OnDataUpdated.Broadcast( Position, Rotation );
        
        // Get button info from module instance
        int Buttons = FPSMove::Get().GetButtons(PSMoveID);
        int Pressed = FPSMove::Get().GetPressed(PSMoveID);
        int Released = FPSMove::Get().GetReleased(PSMoveID);
        
        // Update button member variables
        TriangleDown = (Buttons & PSMove_Button::Btn_TRIANGLE) != 0;
        CircleDown = (Buttons & PSMove_Button::Btn_CIRCLE) != 0;
        CrossDown = (Buttons & PSMove_Button::Btn_CROSS) != 0;
        SquareDown = (Buttons & PSMove_Button::Btn_SQUARE) != 0;
        SelectDown = (Buttons & PSMove_Button::Btn_SELECT) != 0;
        StartDown = (Buttons & PSMove_Button::Btn_START) != 0;
        PSDown = (Buttons & PSMove_Button::Btn_PS) != 0;
        MoveDown = (Buttons & PSMove_Button::Btn_MOVE) != 0;
        
        // Fire button event elegates
        if (TriangleDown || (Released & PSMove_Button::Btn_TRIANGLE))
        {
            OnTriangleButton.Broadcast(TriangleDown, (Pressed & PSMove_Button::Btn_TRIANGLE) != 0, (Released & PSMove_Button::Btn_TRIANGLE) != 0);
        }
        if (CircleDown || (Released & PSMove_Button::Btn_CIRCLE))
        {
            OnCircleButton.Broadcast(CircleDown, (Pressed & PSMove_Button::Btn_CIRCLE) != 0, (Released & PSMove_Button::Btn_CIRCLE) != 0);
        }
        if (CrossDown || (Released & PSMove_Button::Btn_CROSS))
        {
            OnCrossButton.Broadcast(CrossDown, (Pressed & PSMove_Button::Btn_CROSS) != 0, (Released & PSMove_Button::Btn_CROSS) != 0);
        }
        if (SquareDown || (Released & PSMove_Button::Btn_SQUARE))
        {
            OnSquareButton.Broadcast(SquareDown, (Pressed & PSMove_Button::Btn_SQUARE) != 0, (Released & PSMove_Button::Btn_SQUARE) != 0);
        }
        if (SelectDown || (Released & PSMove_Button::Btn_SELECT))
        {
            OnSelectButton.Broadcast(SelectDown, (Pressed & PSMove_Button::Btn_SELECT) != 0, (Released & PSMove_Button::Btn_SELECT) != 0);
        }
        if (StartDown || (Released && PSMove_Button::Btn_START))
        {
            OnStartButton.Broadcast(StartDown, (Pressed & PSMove_Button::Btn_START) != 0, (Released & PSMove_Button::Btn_START) != 0);
        }
        if (PSDown || (Released & PSMove_Button::Btn_PS))
        {
            OnPSButton.Broadcast(PSDown, (Pressed & PSMove_Button::Btn_PS) != 0, (Released & PSMove_Button::Btn_PS) != 0);
        }
        if (MoveDown || (Released && PSMove_Button::Btn_MOVE))
        {
            OnMoveButton.Broadcast(MoveDown, (Pressed & PSMove_Button::Btn_MOVE) != 0, (Released & PSMove_Button::Btn_MOVE) != 0);
        }
        
        // Trigger
        TriggerValue = FPSMove::Get().GetTrigger(PSMoveID);
        OnTriggerButton.Broadcast(TriggerValue, (Pressed & PSMove_Button::Btn_T) != 0, (Released & PSMove_Button::Btn_T) != 0);
        
        // Rumble
        FPSMove::Get().SetRumble(PSMoveID, RumbleRequest);
    }
}
#include "PSMovePrivatePCH.h"
#include "PSMoveComponent.h"
#include "FPSMove.h"

UPSMoveComponent::UPSMoveComponent(const FObjectInitializer &init)
: PSMoveID(0)//, RumbleRequest(0)
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
        TriangleDown = Buttons & PSMove_Button::Btn_TRIANGLE;
        CircleDown = Buttons & PSMove_Button::Btn_CIRCLE;
        CrossDown = Buttons & PSMove_Button::Btn_CROSS;
        SquareDown = Buttons & PSMove_Button::Btn_SQUARE;
        SelectDown = Buttons & PSMove_Button::Btn_SELECT;
        StartDown = Buttons & PSMove_Button::Btn_START;
        PSDown = Buttons & PSMove_Button::Btn_PS;
        MoveDown = Buttons & PSMove_Button::Btn_MOVE;
        
        // Fire button event elegates
        if (TriangleDown || (Released & PSMove_Button::Btn_TRIANGLE))
        {
            OnTriangleButton.Broadcast(TriangleDown, Pressed & PSMove_Button::Btn_TRIANGLE, Released & PSMove_Button::Btn_TRIANGLE);
        }
        if (CircleDown || (Released & PSMove_Button::Btn_CIRCLE))
        {
            OnCircleButton.Broadcast(CircleDown, Pressed & PSMove_Button::Btn_CIRCLE, Released & PSMove_Button::Btn_CIRCLE);
        }
        if (CrossDown || (Released & PSMove_Button::Btn_CROSS))
        {
            OnCrossButton.Broadcast(CrossDown, Pressed & PSMove_Button::Btn_CROSS, Released & PSMove_Button::Btn_CROSS);
        }
        if (SquareDown || (Released & PSMove_Button::Btn_SQUARE))
        {
            OnSquareButton.Broadcast(SquareDown, Pressed & PSMove_Button::Btn_SQUARE, Released & PSMove_Button::Btn_SQUARE);
        }
        if (SelectDown || (Released & PSMove_Button::Btn_SELECT))
        {
            OnSelectButton.Broadcast(SelectDown, Pressed & PSMove_Button::Btn_SELECT, Released & PSMove_Button::Btn_SELECT);
        }
        if (StartDown || (Released && PSMove_Button::Btn_START))
        {
            OnStartButton.Broadcast(StartDown, Pressed & PSMove_Button::Btn_START, Released & PSMove_Button::Btn_START);
        }
        if (PSDown || (Released & PSMove_Button::Btn_PS))
        {
            OnPSButton.Broadcast(PSDown, Pressed & PSMove_Button::Btn_PS, Released & PSMove_Button::Btn_PS);
        }
        if (MoveDown || (Released && PSMove_Button::Btn_MOVE))
        {
            OnMoveButton.Broadcast(MoveDown, Pressed & PSMove_Button::Btn_MOVE, Released & PSMove_Button::Btn_MOVE);
        }
        
        // Trigger
        TriggerValue = FPSMove::Get().GetTrigger(PSMoveID);
        OnTriggerButton.Broadcast(TriggerValue, Pressed & PSMove_Button::Btn_T, Released & PSMove_Button::Btn_T);
        
        // Rumble
        FPSMove::Get().SetRumble(PSMoveID, RumbleRequest);
    }
}
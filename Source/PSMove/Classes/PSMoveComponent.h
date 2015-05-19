#pragma once

#include "PSMoveTypes.h"
#include "FPSMove.h"
#include "PSMoveComponent.generated.h"

UCLASS(ClassGroup="Input Controller", meta=(BlueprintSpawnableComponent))
class UPSMoveComponent : public USceneComponent//, public IPSMoveInterface
{
    GENERATED_UCLASS_BODY()

public:

    //Sets default values for properties.
    UPSMoveComponent();

    virtual void InitializeComponent() override;

    virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

    // PSMove controller ID - 0-based
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    int32 PSMoveID;
 
    UPROPERTY()
    FPSMoveDataFrame DataFrame;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    uint8 RumbleRequest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    bool ResetPoseRequest;

    // Delegate triggered once per frame update
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveDataUpdatedDelegate OnDataUpdated;
    
    // Delegates for buttons. Triggered once per frame if button is down or changes state
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveTriangleButtonDelegate OnTriangleButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveCircleButtonDelegate OnCircleButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveCrossButtonDelegate OnCrossButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveSquareButtonDelegate OnSquareButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveSelectButtonDelegate OnSelectButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveStartButtonDelegate OnStartButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMovePSButtonDelegate OnPSButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveMoveButtonDelegate OnMoveButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveTriggerButtonDelegate OnTriggerButton;
    
protected:
    
};
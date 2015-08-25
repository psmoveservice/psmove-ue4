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

    virtual void BeginPlay() override;
    
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
    
    // Player Index - 0-based
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    int32 PlayerIndex;

    // PSMove controller ID - 0-based
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    int32 PSMoveID;
 
    UPROPERTY()
    FPSMoveDataContext DataContext;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    uint8 RumbleRequest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    FLinearColor LedRequest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    bool ResetPoseRequest;

    UFUNCTION(BlueprintCallable, Category = PSMove)
    void ResetYaw();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
    bool UseHMDCorrection;

    // Delegate triggered once per frame update
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveDataUpdatedDelegate OnDataUpdated;
    
    // Delegates for buttons.
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnTriangleButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnCircleButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnCrossButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnSquareButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnSelectButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnStartButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnPSButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveButtonStateDelegate OnMoveButton;
    
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveTriggerButtonDelegate OnTriggerButton;
    
protected:

private:
    FQuat ZeroYaw;
    FVector LastPosition;
    FQuat LastOrientation;
};
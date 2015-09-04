#pragma once

#include "PSMoveTypes.h"
#include "FPSMove.h"
#include "IMotionController.h"
#include "PSMoveComponent.generated.h"

UCLASS(ClassGroup="Input Controller", meta=(BlueprintSpawnableComponent))
class UPSMoveComponent : public USceneComponent
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

    // Whether this is the right or the left hand of the player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
	TEnumAsByte<EControllerHand> Hand;

	// Set the position and orientation of the actor on update
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PSMove)
	bool ApplyTransformToActor;
    
    UFUNCTION(BlueprintCallable, Category = PSMove)
    void ResetPose();

    UFUNCTION(BlueprintCallable, Category = PSMove)
    void ResetYaw();

    UFUNCTION(BlueprintCallable, Category = PSMove)
    void CycleColours();
    
private:
    FPSMoveDataContext *DataContextPtr;
};
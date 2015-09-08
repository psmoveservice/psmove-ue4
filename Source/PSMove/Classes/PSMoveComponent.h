#pragma once

#include "PSMoveTypes.h"
#include "FPSMove.h"
#include "IMotionController.h"
#include "PSMoveComponent.generated.h"

UCLASS(MinimalAPI, meta = (BlueprintSpawnableComponent), ClassGroup = MotionController)
class UPSMoveComponent : public USceneComponent
{
    GENERATED_BODY()

public:

    //Sets default values for properties.
    UPSMoveComponent(const FObjectInitializer &init);

    virtual void BeginPlay() override;
    
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
        
    // Player Index - 0-based
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSMove")
    int32 PlayerIndex;

    // Whether this is the right or the left hand of the player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PSMove")
	TEnumAsByte<EControllerHand> Hand;

    UFUNCTION(BlueprintCallable, Category = "PSMove")
    void ResetPose();

    UFUNCTION(BlueprintCallable, Category = "PSMove")
    void ResetYaw();

    UFUNCTION(BlueprintCallable, Category = "PSMove")
    void CycleColours();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
    bool bShowHMDFrustumDebug;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
    bool bShowTrackingDebug;
    
private:
    FPSMoveDataContext *DataContextPtr;
};
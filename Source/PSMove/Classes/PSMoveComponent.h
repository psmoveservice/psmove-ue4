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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PSMove)
    FVector Position;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PSMove)
    FRotator Rotation;

    // Delegate triggered once per frame update
    UPROPERTY(BlueprintAssignable, Category = PSMove)
    FPSMoveDataUpdatedDelegate OnDataUpdated;

protected:

    // Copies the data and calls the update delegate
    void UpdateData( float DeltaSeconds );

    // Called before OnDataUpdated.Broadcast
    virtual void OnDataUpdatedImpl(int32 MoveID, float DeltaSeconds);

};
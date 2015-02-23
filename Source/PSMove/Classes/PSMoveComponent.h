#pragma once

#include "IPSMove.h"
#include "PSMoveComponent.generated.h"

UCLASS(ClassGroup="Input Controller", meta=(BlueprintSpawnableComponent))
class UPSMoveComponent : public UActorComponent//, public IPSMoveInterface
{
    GENERATED_UCLASS_BODY()

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="PSMove")
    const FVector GetPosition() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="PSMove")
    const FQuat GetOrientation() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="PSMove")
    const FRotator GetRotator() const;

};
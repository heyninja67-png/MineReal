#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCRCreatureActor.generated.h"
class UStaticMeshComponent; class USceneComponent;
UENUM(BlueprintType) enum class EMCRCreatureType : uint8 { Sheep, Cow, Chicken, Boar };
UCLASS()
class MINECRAFTBEDROCKREALISTIC_API AMCRCreatureActor : public AActor
{
    GENERATED_BODY()
public:
    AMCRCreatureActor();
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Creature") void SetCreatureType(EMCRCreatureType Type);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Creature") float WalkSpeed = 95.0f;
private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Head;
    UPROPERTY(VisibleAnywhere) TArray<TObjectPtr<UStaticMeshComponent>> Legs;
    EMCRCreatureType CreatureType = EMCRCreatureType::Sheep;
    FVector MoveDirection = FVector::ForwardVector;
    float DirectionTimer = 0.0f;
    float AnimationTime = 0.0f;
    float VocalTimer = 12.0f;
    void ChooseDirection();
};

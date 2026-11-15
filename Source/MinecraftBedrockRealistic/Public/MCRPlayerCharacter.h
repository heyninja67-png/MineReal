#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MCRBlockTypes.h"
#include "MCRPlayerCharacter.generated.h"

class UCameraComponent;
class UMCRMobileHUD;
class UMCRInventoryComponent;
class UMCRCraftingComponent;
class UMCRSurvivalComponent;
class AMCRWorldManager;
class USoundBase;

UCLASS()
class MINECRAFTBEDROCKREALISTIC_API AMCRPlayerCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AMCRPlayerCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category="Voxel") void BreakBlock();
    UFUNCTION(BlueprintCallable, Category="Voxel") void PlaceBlock();
    UFUNCTION(BlueprintCallable, Category="Inventory") void SelectNextHotbarSlot();
    UFUNCTION(BlueprintCallable, Category="Inventory") void ToggleInventory();
    UFUNCTION(BlueprintCallable, Category="Player") void ToggleCreativeMode();
    UFUNCTION(BlueprintCallable, Category="Player") void PressJump();
    UFUNCTION(BlueprintPure, Category="Inventory") UMCRInventoryComponent* GetInventoryComponent() const { return Inventory; }
    UFUNCTION(BlueprintPure, Category="Inventory") UMCRCraftingComponent* GetCraftingComponent() const { return Crafting; }
    UFUNCTION(BlueprintPure, Category="Survival") UMCRSurvivalComponent* GetSurvivalComponent() const { return Survival; }

protected:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void BeginCrouch();
    void EndCrouch();
    bool TraceTarget(FHitResult& Hit) const;
    FIntVector HitToBlock(const FHitResult& Hit, bool bAdjacent) const;

private:
    UPROPERTY(VisibleAnywhere, Category="Camera") TObjectPtr<UCameraComponent> PlayerCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true")) TObjectPtr<UMCRInventoryComponent> Inventory;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true")) TObjectPtr<UMCRCraftingComponent> Crafting;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true")) TObjectPtr<UMCRSurvivalComponent> Survival;
    UPROPERTY() TObjectPtr<UMCRMobileHUD> MobileHUD;
    TWeakObjectPtr<AMCRWorldManager> WorldManager;
    bool bTrackingFall = false;
    float FallStartZ = 0.0f;
    float FootstepAccumulator = 0.0f;
    UPROPERTY() TObjectPtr<USoundBase> FootstepSound;
};

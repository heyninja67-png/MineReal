#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCRBlockTypes.h"
#include "MCRInventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FMCRItemStack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    EMCRBlock BlockType = EMCRBlock::Air;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta=(ClampMin="0", ClampMax="999"))
    int32 Count = 0;

    bool IsEmpty() const { return BlockType == EMCRBlock::Air || Count <= 0; }
};

UCLASS(ClassGroup=(MCR), meta=(BlueprintSpawnableComponent))
class MINECRAFTBEDROCKREALISTIC_API UMCRInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMCRInventoryComponent();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    TArray<FMCRItemStack> Slots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
    int32 SelectedHotbarSlot = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory", meta=(ClampMin="9", ClampMax="72"))
    int32 InventoryCapacity = 36;

    UFUNCTION(BlueprintCallable, Category="Inventory") int32 AddBlock(EMCRBlock Block, int32 Count);
    UFUNCTION(BlueprintCallable, Category="Inventory") bool RemoveBlock(EMCRBlock Block, int32 Count);
    UFUNCTION(BlueprintCallable, Category="Inventory") bool ConsumeSlot(int32 SlotIndex, int32 Count = 1);
    UFUNCTION(BlueprintCallable, Category="Inventory") bool SetSlot(int32 SlotIndex, EMCRBlock Block, int32 Count);
    UFUNCTION(BlueprintCallable, Category="Inventory") bool MoveOrMergeSlots(int32 FromIndex, int32 ToIndex);
    UFUNCTION(BlueprintCallable, Category="Inventory") bool SplitHalf(int32 FromIndex, int32 ToIndex);
    UFUNCTION(BlueprintCallable, Category="Inventory") void SelectHotbarSlot(int32 SlotIndex);
    UFUNCTION(BlueprintCallable, Category="Inventory") void SelectNextHotbarSlot();
    UFUNCTION(BlueprintPure, Category="Inventory") int32 CountBlock(EMCRBlock Block) const;
    UFUNCTION(BlueprintPure, Category="Inventory") FMCRItemStack GetSelectedStack() const;
    UFUNCTION(BlueprintCallable, Category="Inventory") void InitializeStarterKit();

    int32 GetHotbarSize() const { return 9; }
    static int32 MaxStackFor(EMCRBlock Block);
};

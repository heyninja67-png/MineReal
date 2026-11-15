#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCRInventoryWidget.generated.h"
class UButton; class UTextBlock; class UMCRInventoryComponent; class UMCRCraftingComponent;
UCLASS()
class MINECRAFTBEDROCKREALISTIC_API UMCRInventoryWidget : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeConstruct() override;
private:
    UPROPERTY() TObjectPtr<UMCRInventoryComponent> Inventory;
    UPROPERTY() TObjectPtr<UMCRCraftingComponent> Crafting;
    UPROPERTY() TArray<TObjectPtr<UButton>> SlotButtons;
    UPROPERTY() TObjectPtr<UTextBlock> InfoText;
    int32 PendingSlot = INDEX_NONE;
    void RefreshSlots();
    void OnSlotClicked(int32 Index);
    void SplitPendingStack();
    void CloseInventory();
};

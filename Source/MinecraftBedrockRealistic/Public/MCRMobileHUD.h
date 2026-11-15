#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCRMobileHUD.generated.h"
class UCanvasPanel; class UButton; class UTextBlock; class UUniformGridPanel; class UMCRInventoryWidget;
UCLASS()
class MINECRAFTBEDROCKREALISTIC_API UMCRMobileHUD : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual void NativeConstruct() override;
public:
    UFUNCTION(BlueprintCallable, Category="UI") void ToggleInventory();
protected:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
    UFUNCTION() void OnMineClicked();
    UFUNCTION() void OnPlaceClicked();
    UFUNCTION() void OnJumpClicked();
    UFUNCTION() void OnNextSlotClicked();
    UFUNCTION() void OnInventoryClicked();
    UFUNCTION() void OnCreativeClicked();
    UButton* AddActionButton(UCanvasPanel* Canvas, const FString& Label, const FVector2D& Position, const FName& Name);
    void RefreshHUD();
    UPROPERTY() TObjectPtr<UTextBlock> StatusText;
    UPROPERTY() TObjectPtr<UUniformGridPanel> HotbarGrid;
    UPROPERTY() TArray<TObjectPtr<UButton>> HotbarButtons;
    UPROPERTY() TObjectPtr<UMCRInventoryWidget> InventoryWidget;
};

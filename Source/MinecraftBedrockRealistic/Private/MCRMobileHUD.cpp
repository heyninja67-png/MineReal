#include "MCRMobileHUD.h"
#include "MCRPlayerCharacter.h"
#include "MCRInventoryComponent.h"
#include "MCRSurvivalComponent.h"
#include "MCRInventoryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

void UMCRMobileHUD::NativeConstruct()
{
    Super::NativeConstruct();
    if (WidgetTree->RootWidget) return;
    UCanvasPanel* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),TEXT("MobileRoot")); WidgetTree->RootWidget=Canvas;
    StatusText=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("Vitals"));
    StatusText->SetColorAndOpacity(FSlateColor(FLinearColor::White)); StatusText->SetShadowOffset(FVector2D(1,1));
    UCanvasPanelSlot* StatusSlot=Canvas->AddChildToCanvas(StatusText); StatusSlot->SetAnchors(FAnchors(0,0)); StatusSlot->SetPosition(FVector2D(22,18)); StatusSlot->SetSize(FVector2D(360,48));
    HotbarGrid=WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(),TEXT("Hotbar"));
    UCanvasPanelSlot* GridSlot=Canvas->AddChildToCanvas(HotbarGrid); GridSlot->SetAnchors(FAnchors(.5f,1.0f)); GridSlot->SetAlignment(FVector2D(.5f,1.0f)); GridSlot->SetPosition(FVector2D(0,-12)); GridSlot->SetSize(FVector2D(580,66));
    HotbarButtons.Reset();
    for (int32 I=0;I<9;++I)
    {
        UButton* Button=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),*FString::Printf(TEXT("Hotbar_%d"),I));
        UTextBlock* Label=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); Label->SetText(FText::FromString(TEXT("-"))); Label->SetJustification(ETextJustify::Center); Label->SetColorAndOpacity(FSlateColor(FLinearColor::White)); Button->AddChild(Label);
        UUniformGridSlot* Slot=HotbarGrid->AddChildToUniformGrid(Button,0,I); Slot->SetPadding(FMargin(2));
        TWeakObjectPtr<UMCRMobileHUD> WeakThis(this); Button->OnClicked.AddLambda([WeakThis,I](){ if (WeakThis.IsValid()) if (AMCRPlayerCharacter* P=Cast<AMCRPlayerCharacter>(WeakThis->GetOwningPlayerPawn())) if (P->GetInventoryComponent()) P->GetInventoryComponent()->SelectHotbarSlot(I); });
        HotbarButtons.Add(Button);
    }
    UButton* Mine=AddActionButton(Canvas,TEXT("MINAR"),FVector2D(-150,-252),TEXT("MineButton"));
    UButton* Place=AddActionButton(Canvas,TEXT("COLOCAR"),FVector2D(-295,-252),TEXT("PlaceButton"));
    UButton* JumpButton=AddActionButton(Canvas,TEXT("PULAR"),FVector2D(-150,-180),TEXT("JumpButton"));
    UButton* Next=AddActionButton(Canvas,TEXT("ITEM +"),FVector2D(-295,-180),TEXT("NextSlotButton"));
    UButton* Inventory=AddActionButton(Canvas,TEXT("MOCHILA"),FVector2D(-295,-108),TEXT("InventoryButton"));
    UButton* Creative=AddActionButton(Canvas,TEXT("CRIATIVO"),FVector2D(-150,-108),TEXT("CreativeButton"));
    Mine->OnClicked.AddDynamic(this,&UMCRMobileHUD::OnMineClicked); Place->OnClicked.AddDynamic(this,&UMCRMobileHUD::OnPlaceClicked);
    JumpButton->OnClicked.AddDynamic(this,&UMCRMobileHUD::OnJumpClicked); Next->OnClicked.AddDynamic(this,&UMCRMobileHUD::OnNextSlotClicked);
    Inventory->OnClicked.AddDynamic(this,&UMCRMobileHUD::OnInventoryClicked); Creative->OnClicked.AddDynamic(this,&UMCRMobileHUD::OnCreativeClicked);
    RefreshHUD();
}
void UMCRMobileHUD::NativeTick(const FGeometry& MyGeometry,float InDeltaTime) { Super::NativeTick(MyGeometry,InDeltaTime); RefreshHUD(); }
UButton* UMCRMobileHUD::AddActionButton(UCanvasPanel* Canvas,const FString& Label,const FVector2D& Position,const FName& Name)
{
    UButton* Button=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),Name); Button->SetBackgroundColor(FLinearColor(.025f,.045f,.065f,.82f));
    UTextBlock* Text=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); Text->SetText(FText::FromString(Label)); Text->SetColorAndOpacity(FSlateColor(FLinearColor::White)); Text->SetJustification(ETextJustify::Center); Button->AddChild(Text);
    UCanvasPanelSlot* Slot=Canvas->AddChildToCanvas(Button); Slot->SetAnchors(FAnchors(1,1)); Slot->SetAlignment(FVector2D(1,1)); Slot->SetPosition(Position); Slot->SetSize(FVector2D(132,58)); return Button;
}
void UMCRMobileHUD::RefreshHUD()
{
    AMCRPlayerCharacter* Player=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn()); if (!Player) return;
    if (StatusText && Player->GetSurvivalComponent()) { const UMCRSurvivalComponent* S=Player->GetSurvivalComponent(); StatusText->SetText(FText::FromString(FString::Printf(TEXT("VIDA  %.0f/20     FOME  %.0f/20"),S->Health,S->Hunger))); }
    UMCRInventoryComponent* Inv=Player->GetInventoryComponent(); if (!Inv) return;
    for (int32 I=0;I<HotbarButtons.Num() && Inv->Slots.IsValidIndex(I);++I)
    {
        if (UButton* Button=HotbarButtons[I])
        {
            if (UTextBlock* Label=Cast<UTextBlock>(Button->GetContent()))
            {
                const FMCRItemStack& Stack=Inv->Slots[I];
                FString Name=Stack.IsEmpty()?TEXT("-"):StaticEnum<EMCRBlock>()->GetDisplayNameTextByValue(static_cast<int64>(Stack.BlockType)).ToString();
                Label->SetText(FText::FromString(FString::Printf(TEXT("%s\n%d"),*Name,Stack.Count)));
            }
            Button->SetBackgroundColor(I==Inv->SelectedHotbarSlot?FLinearColor(.18f,.39f,.48f,.94f):FLinearColor(.025f,.045f,.065f,.8f));
        }
    }
}
void UMCRMobileHUD::OnMineClicked(){if (AMCRPlayerCharacter* P=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn()))P->BreakBlock();}
void UMCRMobileHUD::OnPlaceClicked(){if (AMCRPlayerCharacter* P=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn()))P->PlaceBlock();}
void UMCRMobileHUD::OnJumpClicked(){if (AMCRPlayerCharacter* P=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn()))P->PressJump();}
void UMCRMobileHUD::OnNextSlotClicked(){if (AMCRPlayerCharacter* P=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn()))P->SelectNextHotbarSlot();}
void UMCRMobileHUD::OnInventoryClicked(){ToggleInventory();}
void UMCRMobileHUD::OnCreativeClicked(){if (AMCRPlayerCharacter* P=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn())) P->ToggleCreativeMode();}
void UMCRMobileHUD::ToggleInventory()
{
    if (InventoryWidget && InventoryWidget->IsInViewport()) { InventoryWidget->RemoveFromParent(); return; }
    if (APlayerController* PC=GetOwningPlayer()) { InventoryWidget=CreateWidget<UMCRInventoryWidget>(PC,UMCRInventoryWidget::StaticClass()); if (InventoryWidget) InventoryWidget->AddToViewport(20); }
}

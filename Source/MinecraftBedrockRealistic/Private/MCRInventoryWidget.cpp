#include "MCRInventoryWidget.h"
#include "MCRPlayerCharacter.h"
#include "MCRInventoryComponent.h"
#include "MCRCraftingComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"

void UMCRInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (AMCRPlayerCharacter* Player=Cast<AMCRPlayerCharacter>(GetOwningPlayerPawn())) { Inventory=Player->GetInventoryComponent(); Crafting=Player->GetCraftingComponent(); }
    UCanvasPanel* Root=WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),TEXT("InventoryRoot")); WidgetTree->RootWidget=Root;
    UBorder* Back=WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("Backdrop")); Back->SetBrushColor(FLinearColor(.015f,.025f,.04f,.94f));
    UCanvasPanelSlot* BackSlot=Root->AddChildToCanvas(Back); BackSlot->SetAnchors(FAnchors(.08f,.08f,.92f,.92f)); BackSlot->SetOffsets(FMargin(0));
    UVerticalBox* Panel=WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass()); Back->SetContent(Panel);
    UTextBlock* Title=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); Title->SetText(FText::FromString(TEXT("INVENTÁRIO  •  36 ESPAÇOS"))); Title->SetColorAndOpacity(FSlateColor(FLinearColor::White)); Panel->AddChildToVerticalBox(Title);
    InfoText=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); InfoText->SetText(FText::FromString(TEXT("Toque em um espaço para selecionar; toque em outro para mover ou trocar."))); InfoText->SetColorAndOpacity(FSlateColor(FLinearColor(.7f,.84f,.91f,1))); Panel->AddChildToVerticalBox(InfoText);
    UUniformGridPanel* Grid=WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(),TEXT("SlotGrid")); Panel->AddChildToVerticalBox(Grid);
    SlotButtons.Reset();
    for (int32 I=0; I<36; ++I)
    {
        UButton* Button=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),*FString::Printf(TEXT("Slot_%02d"),I));
        UTextBlock* Label=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); Label->SetText(FText::FromString(TEXT("-"))); Label->SetJustification(ETextJustify::Center); Label->SetColorAndOpacity(FSlateColor(FLinearColor::White)); Button->AddChild(Label);
        UUniformGridSlot* Slot=Grid->AddChildToUniformGrid(Button,I/9,I%9); Slot->SetPadding(FMargin(3));
        TWeakObjectPtr<UMCRInventoryWidget> WeakThis(this); Button->OnClicked.AddLambda([WeakThis,I](){ if (WeakThis.IsValid()) WeakThis->OnSlotClicked(I); }); SlotButtons.Add(Button);
    }
    UTextBlock* CraftTitle=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); CraftTitle->SetText(FText::FromString(TEXT("RECEITAS RÁPIDAS"))); CraftTitle->SetColorAndOpacity(FSlateColor(FLinearColor(.92f,.78f,.40f,1))); Panel->AddChildToVerticalBox(CraftTitle);
    const TPair<FName,FString> RecipeButtons[] = {
        {TEXT("wood_to_planks"),TEXT("1 Madeira → 4 Tábuas")},
        {TEXT("planks_to_table"),TEXT("4 Tábuas → Bancada")},
        {TEXT("stone_to_cobble"),TEXT("1 Pedra → Pedregulho")},
        {TEXT("sand_to_glass"),TEXT("4 Areias → Vidro")},
        {TEXT("cobble_to_furnace"),TEXT("8 Pedregulhos → Fornalha")},
        {TEXT("planks_to_chest"),TEXT("8 Tábuas → Baú")},
        {TEXT("coal_to_torches"),TEXT("1 Carvão + 1 Madeira → 4 Tochas")}
    };
    for (const TPair<FName,FString>& Recipe : RecipeButtons)
    {
        UButton* RecipeButton=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
        UTextBlock* RecipeLabel=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); RecipeLabel->SetText(FText::FromString(Recipe.Value)); RecipeLabel->SetJustification(ETextJustify::Center); RecipeLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White)); RecipeButton->AddChild(RecipeLabel);
        Panel->AddChildToVerticalBox(RecipeButton);
        const FName RecipeId=Recipe.Key; TWeakObjectPtr<UMCRInventoryWidget> WeakThis(this);
        RecipeButton->OnClicked.AddLambda([WeakThis,RecipeId](){ if (WeakThis.IsValid() && WeakThis->Crafting) { WeakThis->Crafting->Craft(RecipeId); WeakThis->RefreshSlots(); } });
    }
    UHorizontalBox* Actions=WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass()); Panel->AddChildToVerticalBox(Actions);
    UButton* Split=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass()); UTextBlock* SplitLabel=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); SplitLabel->SetText(FText::FromString(TEXT("DIVIDIR PILHA"))); SplitLabel->SetJustification(ETextJustify::Center); Split->AddChild(SplitLabel); Actions->AddChildToHorizontalBox(Split); Split->OnClicked.AddLambda([this](){SplitPendingStack();});
    UButton* Close=WidgetTree->ConstructWidget<UButton>(UButton::StaticClass()); UTextBlock* CloseLabel=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass()); CloseLabel->SetText(FText::FromString(TEXT("FECHAR"))); CloseLabel->SetJustification(ETextJustify::Center); Close->AddChild(CloseLabel); Actions->AddChildToHorizontalBox(Close); Close->OnClicked.AddLambda([this](){CloseInventory();});
    RefreshSlots();
}
void UMCRInventoryWidget::RefreshSlots()
{
    if (!Inventory) return;
    const TArray<FMCRItemStack>& Slots=Inventory->Slots;
    for (int32 I=0; I<SlotButtons.Num() && Slots.IsValidIndex(I); ++I)
    {
        if (UButton* Button=SlotButtons[I])
        {
            if (UTextBlock* Label=Cast<UTextBlock>(Button->GetContent()))
            {
                const FMCRItemStack& Stack=Slots[I];
                const FString Name=Stack.IsEmpty()?TEXT("VAZIO"):StaticEnum<EMCRBlock>()->GetDisplayNameTextByValue(static_cast<int64>(Stack.BlockType)).ToString();
                Label->SetText(FText::FromString(FString::Printf(TEXT("%02d\n%s\nx%d"),I+1,*Name,Stack.Count)));
            }
            Button->SetBackgroundColor(I==PendingSlot?FLinearColor(.18f,.42f,.50f,.95f):FLinearColor(.07f,.10f,.14f,.92f));
        }
    }
}
void UMCRInventoryWidget::OnSlotClicked(int32 Index)
{
    if (!Inventory) return;
    if (PendingSlot==INDEX_NONE) { PendingSlot=Index; }
    else { Inventory->MoveOrMergeSlots(PendingSlot,Index); PendingSlot=INDEX_NONE; }
    RefreshSlots();
}
void UMCRInventoryWidget::SplitPendingStack()
{
    if (!Inventory || PendingSlot==INDEX_NONE) return;
    for (int32 I=9; I<Inventory->Slots.Num(); ++I)
    {
        if (Inventory->Slots[I].IsEmpty() && Inventory->SplitHalf(PendingSlot,I)) break;
    }
    RefreshSlots();
}
void UMCRInventoryWidget::CloseInventory() { RemoveFromParent(); }

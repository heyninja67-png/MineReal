#include "MCRInventoryComponent.h"

UMCRInventoryComponent::UMCRInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    Slots.SetNum(36);
}

int32 UMCRInventoryComponent::MaxStackFor(EMCRBlock Block)
{
    if (Block == EMCRBlock::Air || Block == EMCRBlock::Water) return 0;
    if (Block == EMCRBlock::Glass || Block == EMCRBlock::Ice) return 32;
    return 64;
}

int32 UMCRInventoryComponent::AddBlock(EMCRBlock Block, int32 Count)
{
    if (Block == EMCRBlock::Air || Count <= 0) return Count;
    int32 Remaining = Count;
    const int32 MaxStack = MaxStackFor(Block);
    for (FMCRItemStack& Stack : Slots)
    {
        if (Stack.BlockType == Block && Stack.Count < MaxStack)
        {
            const int32 Added = FMath::Min(Remaining, MaxStack - Stack.Count);
            Stack.Count += Added;
            Remaining -= Added;
            if (Remaining <= 0) return 0;
        }
    }
    for (FMCRItemStack& Stack : Slots)
    {
        if (Stack.IsEmpty())
        {
            const int32 Added = FMath::Min(Remaining, MaxStack);
            Stack.BlockType = Block;
            Stack.Count = Added;
            Remaining -= Added;
            if (Remaining <= 0) return 0;
        }
    }
    return Remaining;
}

bool UMCRInventoryComponent::RemoveBlock(EMCRBlock Block, int32 Count)
{
    if (Count <= 0 || CountBlock(Block) < Count) return false;
    int32 Remaining = Count;
    for (FMCRItemStack& Stack : Slots)
    {
        if (Stack.BlockType != Block) continue;
        const int32 Removed = FMath::Min(Remaining, Stack.Count);
        Stack.Count -= Removed;
        Remaining -= Removed;
        if (Stack.Count <= 0) Stack = FMCRItemStack();
        if (Remaining <= 0) return true;
    }
    return Remaining <= 0;
}

bool UMCRInventoryComponent::ConsumeSlot(int32 SlotIndex, int32 Count)
{
    if (!Slots.IsValidIndex(SlotIndex) || Count <= 0 || Slots[SlotIndex].Count < Count) return false;
    Slots[SlotIndex].Count -= Count;
    if (Slots[SlotIndex].Count <= 0) Slots[SlotIndex] = FMCRItemStack();
    return true;
}

bool UMCRInventoryComponent::SetSlot(int32 SlotIndex, EMCRBlock Block, int32 Count)
{
    if (!Slots.IsValidIndex(SlotIndex) || Count < 0 || Count > MaxStackFor(Block)) return false;
    if (Count == 0 || Block == EMCRBlock::Air) Slots[SlotIndex] = FMCRItemStack();
    else { Slots[SlotIndex].BlockType = Block; Slots[SlotIndex].Count = Count; }
    return true;
}

bool UMCRInventoryComponent::MoveOrMergeSlots(int32 FromIndex, int32 ToIndex)
{
    if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex) || FromIndex == ToIndex) return false;
    FMCRItemStack& From = Slots[FromIndex];
    FMCRItemStack& To = Slots[ToIndex];
    if (From.IsEmpty()) return false;
    if (To.IsEmpty()) { To = From; From = FMCRItemStack(); return true; }
    if (To.BlockType == From.BlockType)
    {
        const int32 Added = FMath::Min(From.Count, MaxStackFor(To.BlockType) - To.Count);
        if (Added <= 0) return false;
        To.Count += Added;
        From.Count -= Added;
        if (From.Count <= 0) From = FMCRItemStack();
        return true;
    }
    Swap(From, To);
    return true;
}

bool UMCRInventoryComponent::SplitHalf(int32 FromIndex, int32 ToIndex)
{
    if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex) || FromIndex == ToIndex) return false;
    FMCRItemStack& From = Slots[FromIndex];
    FMCRItemStack& To = Slots[ToIndex];
    if (From.IsEmpty() || !To.IsEmpty() || From.Count < 2) return false;
    const int32 SplitCount = From.Count / 2;
    To.BlockType = From.BlockType;
    To.Count = SplitCount;
    From.Count -= SplitCount;
    return true;
}

void UMCRInventoryComponent::SelectHotbarSlot(int32 SlotIndex)
{
    SelectedHotbarSlot = FMath::Clamp(SlotIndex, 0, 8);
}

void UMCRInventoryComponent::SelectNextHotbarSlot()
{
    SelectedHotbarSlot = (SelectedHotbarSlot + 1) % 9;
}

int32 UMCRInventoryComponent::CountBlock(EMCRBlock Block) const
{
    int32 Total = 0;
    for (const FMCRItemStack& Stack : Slots) if (Stack.BlockType == Block) Total += Stack.Count;
    return Total;
}

FMCRItemStack UMCRInventoryComponent::GetSelectedStack() const
{
    return Slots.IsValidIndex(SelectedHotbarSlot) ? Slots[SelectedHotbarSlot] : FMCRItemStack();
}

void UMCRInventoryComponent::InitializeStarterKit()
{
    if (Slots.Num() != InventoryCapacity) Slots.SetNum(InventoryCapacity);
    if (!CountBlock(EMCRBlock::Stone) && !CountBlock(EMCRBlock::Dirt))
    {
        SetSlot(0, EMCRBlock::Stone, 32);
        SetSlot(1, EMCRBlock::Dirt, 32);
        SetSlot(2, EMCRBlock::Grass, 16);
        SetSlot(3, EMCRBlock::Wood, 24);
        SetSlot(4, EMCRBlock::Leaves, 24);
        SetSlot(5, EMCRBlock::Sand, 24);
        SetSlot(6, EMCRBlock::Cobblestone, 32);
        SetSlot(7, EMCRBlock::Planks, 32);
        SetSlot(8, EMCRBlock::Torch, 16);
    }
}

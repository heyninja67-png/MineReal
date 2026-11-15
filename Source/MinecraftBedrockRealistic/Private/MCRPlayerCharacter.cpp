#include "MCRPlayerCharacter.h"
#include "MCRWorldManager.h"
#include "MCRMobileHUD.h"
#include "MCRInventoryComponent.h"
#include "MCRCraftingComponent.h"
#include "MCRSurvivalComponent.h"
#include "MCRWorldSaveGame.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AMCRPlayerCharacter::AMCRPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = true;
    bUseControllerRotationRoll = false;

    Inventory = CreateDefaultSubobject<UMCRInventoryComponent>(TEXT("Inventory"));
    Crafting = CreateDefaultSubobject<UMCRCraftingComponent>(TEXT("Crafting"));
    Survival = CreateDefaultSubobject<UMCRSurvivalComponent>(TEXT("Survival"));

    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    PlayerCamera->SetupAttachment(GetCapsuleComponent());
    PlayerCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    PlayerCamera->bUsePawnControlRotation = true;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 520.0f;
    GetCharacterMovement()->JumpZVelocity = 520.0f;
    GetCharacterMovement()->AirControl = 0.25f;
    GetCharacterMovement()->GravityScale = 1.7f;


}

void AMCRPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (GetActorLocation().Z < 1000.0f)
    {
        SetActorLocation(FVector(0.0f, 0.0f, 4200.0f));
    }

    FootstepSound=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/SFX_Footstep_Grass.SFX_Footstep_Grass"));
    if (Inventory)
    {
        Inventory->InitializeStarterKit();
        if (UGameplayStatics::DoesSaveGameExist(TEXT("MCR_World_01"), 0))
        {
            if (UMCRWorldSaveGame* Save = Cast<UMCRWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("MCR_World_01"), 0)))
            {
                if (Save->PlayerInventory.Num() > 0)
                {
                    for (int32 I=0; I<Save->PlayerInventory.Num() && I<Inventory->Slots.Num(); ++I)
                        Inventory->SetSlot(I, Save->PlayerInventory[I].BlockType, Save->PlayerInventory[I].Count);
                }
                if (Survival) { Survival->Health=Save->PlayerHealth; Survival->Hunger=Save->PlayerHunger; }
            }
        }
    }

    for (TActorIterator<AMCRWorldManager> It(GetWorld()); It; ++It)
    {
        WorldManager = *It;
        break;
    }
    if (!WorldManager.IsValid() && GetWorld())
    {
        AMCRWorldManager* NewManager = GetWorld()->SpawnActor<AMCRWorldManager>();
        WorldManager = NewManager;
    }

    if (IsLocallyControlled())
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            MobileHUD = CreateWidget<UMCRMobileHUD>(PC, UMCRMobileHUD::StaticClass());
            if (MobileHUD)
            {
                MobileHUD->AddToViewport(5);
            }
        }
    }
}

void AMCRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMCRPlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMCRPlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AMCRPlayerCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AMCRPlayerCharacter::LookUp);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AMCRPlayerCharacter::PressJump);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AMCRPlayerCharacter::BeginCrouch);
    PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &AMCRPlayerCharacter::EndCrouch);
    PlayerInputComponent->BindAction(TEXT("BreakBlock"), IE_Pressed, this, &AMCRPlayerCharacter::BreakBlock);
    PlayerInputComponent->BindAction(TEXT("PlaceBlock"), IE_Pressed, this, &AMCRPlayerCharacter::PlaceBlock);
    PlayerInputComponent->BindAction(TEXT("NextSlot"), IE_Pressed, this, &AMCRPlayerCharacter::SelectNextHotbarSlot);
    PlayerInputComponent->BindAction(TEXT("Inventory"), IE_Pressed, this, &AMCRPlayerCharacter::ToggleInventory);
    PlayerInputComponent->BindAction(TEXT("ToggleCreative"), IE_Pressed, this, &AMCRPlayerCharacter::ToggleCreativeMode);
}

void AMCRPlayerCharacter::MoveForward(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
    }
}

void AMCRPlayerCharacter::MoveRight(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
    }
}

void AMCRPlayerCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void AMCRPlayerCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void AMCRPlayerCharacter::PressJump()
{
    if (Survival && Survival->bCreativeMode) LaunchCharacter(FVector(0.0f,0.0f,380.0f),false,false);
    else Jump();
}

void AMCRPlayerCharacter::BeginCrouch()
{
    Crouch();
}

void AMCRPlayerCharacter::EndCrouch()
{
    UnCrouch();
}

bool AMCRPlayerCharacter::TraceTarget(FHitResult& Hit) const
{
    if (!PlayerCamera || !GetWorld())
    {
        return false;
    }

    const FVector Start = PlayerCamera->GetComponentLocation();
    const FVector End = Start + PlayerCamera->GetForwardVector() * 650.0f;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(MCRBlockInteraction), false, this);
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
}

FIntVector AMCRPlayerCharacter::HitToBlock(const FHitResult& Hit, bool bAdjacent) const
{
    const FVector Offset = Hit.ImpactNormal * (bAdjacent ? 5.0f : -5.0f);
    const FVector Point = Hit.ImpactPoint + Offset;
    return FIntVector(
        FMath::FloorToInt(Point.X / MCRVoxel::BlockSize),
        FMath::FloorToInt(Point.Y / MCRVoxel::BlockSize),
        FMath::FloorToInt(Point.Z / MCRVoxel::BlockSize));
}

void AMCRPlayerCharacter::BreakBlock()
{
    if (!WorldManager.IsValid())
    {
        return;
    }

    FHitResult Hit;
    if (!TraceTarget(Hit))
    {
        return;
    }

    const FIntVector BlockPosition = HitToBlock(Hit, false);
    const EMCRBlock Block = WorldManager->GetBlockAtWorld(BlockPosition);
    if (Block == EMCRBlock::Air || !WorldManager->SetBlockAtWorld(BlockPosition, EMCRBlock::Air))
    {
        return;
    }

    if (Inventory) Inventory->AddBlock(Block, 1);
    const TCHAR* SoundPath=(Block==EMCRBlock::Wood || Block==EMCRBlock::Leaves || Block==EMCRBlock::Planks)
        ? TEXT("/Game/Audio/SFX_BlockBreak_Wood.SFX_BlockBreak_Wood")
        : TEXT("/Game/Audio/SFX_BlockBreak_Stone.SFX_BlockBreak_Stone");
    if (USoundBase* Sound=LoadObject<USoundBase>(nullptr,SoundPath)) UGameplayStatics::PlaySoundAtLocation(this,Sound,Hit.ImpactPoint);
}

void AMCRPlayerCharacter::PlaceBlock()
{
    if (!WorldManager.IsValid() || !Inventory) return;
    const FMCRItemStack Selected = Inventory->GetSelectedStack();
    if (Selected.IsEmpty()) return;

    FHitResult Hit;
    if (!TraceTarget(Hit))
    {
        return;
    }

    const FIntVector TargetPosition = HitToBlock(Hit, true);
    if (WorldManager->GetBlockAtWorld(TargetPosition) != EMCRBlock::Air)
    {
        return;
    }

    if (WorldManager->SetBlockAtWorld(TargetPosition, Selected.BlockType))
    {
        if (!Survival || !Survival->bCreativeMode) Inventory->ConsumeSlot(Inventory->SelectedHotbarSlot, 1);
        if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/SFX_BlockPlace.SFX_BlockPlace")))
            UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint);
    }
}

void AMCRPlayerCharacter::SelectNextHotbarSlot()
{
    if (Inventory) Inventory->SelectNextHotbarSlot();
}

void AMCRPlayerCharacter::ToggleInventory()
{
    if (MobileHUD) MobileHUD->ToggleInventory();
}

void AMCRPlayerCharacter::ToggleCreativeMode()
{
    if (!Survival) return;
    const bool bEnable=!Survival->bCreativeMode;
    Survival->SetCreativeMode(bEnable);
    GetCharacterMovement()->SetMovementMode(bEnable ? MOVE_Flying : MOVE_Walking);
}

void AMCRPlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!GetCharacterMovement() || !Survival) return;
    const bool bFalling=GetCharacterMovement()->IsFalling();
    if (GetCharacterMovement()->IsMovingOnGround() && GetVelocity().Size2D()>85.0f)
    {
        FootstepAccumulator+=DeltaSeconds;
        if (FootstepAccumulator>=0.43f)
        {
            FootstepAccumulator=0.0f;
            if (FootstepSound) UGameplayStatics::PlaySoundAtLocation(this,FootstepSound,GetActorLocation(),0.24f);
        }
    }
    else FootstepAccumulator=0.0f;
    if (bFalling && !bTrackingFall) { bTrackingFall=true; FallStartZ=GetActorLocation().Z; }
    else if (!bFalling && bTrackingFall)
    {
        bTrackingFall=false;
        Survival->ApplyFallDamage(FallStartZ-GetActorLocation().Z);
    }
}

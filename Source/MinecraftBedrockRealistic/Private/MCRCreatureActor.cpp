#include "MCRCreatureActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

AMCRCreatureActor::AMCRCreatureActor()
{
    PrimaryActorTick.bCanEverTick = true;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root")); SetRootComponent(SceneRoot);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body")); Body->SetupAttachment(SceneRoot); Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head")); Head->SetupAttachment(SceneRoot); Head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CubeMesh.Succeeded()) { Body->SetStaticMesh(CubeMesh.Object); Head->SetStaticMesh(CubeMesh.Object); }
    Body->SetRelativeLocation(FVector(0,0,85)); Body->SetRelativeScale3D(FVector(1.30f,.72f,.76f));
    Head->SetRelativeLocation(FVector(72,0,115)); Head->SetRelativeScale3D(FVector(.48f,.46f,.52f));
    for (int32 I=0; I<4; ++I)
    {
        UStaticMeshComponent* Leg = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Leg_%d"),I)); Leg->SetupAttachment(SceneRoot);
        if (CubeMesh.Succeeded()) Leg->SetStaticMesh(CubeMesh.Object);
        Leg->SetRelativeScale3D(FVector(.18f,.18f,.55f));
        Leg->SetRelativeLocation(FVector(I<2 ? -38.0f:38.0f, I%2==0 ? -24.0f:24.0f, 30.0f));
        Leg->SetCollisionEnabled(ECollisionEnabled::NoCollision); Legs.Add(Leg);
    }
    SetCreatureType(EMCRCreatureType::Sheep);
    VocalTimer=FMath::FRandRange(7.0f,18.0f);
}
void AMCRCreatureActor::SetCreatureType(EMCRCreatureType Type)
{
    CreatureType=Type;
    FLinearColor BodyColor=Type==EMCRCreatureType::Sheep?FLinearColor(.78f,.77f,.70f):Type==EMCRCreatureType::Cow?FLinearColor(.29f,.22f,.17f):Type==EMCRCreatureType::Chicken?FLinearColor(.88f,.83f,.67f):FLinearColor(.36f,.20f,.13f);
    FString MeshPath;
    switch (Type)
    {
        case EMCRCreatureType::Cow: MeshPath=TEXT("/Game/Models/SM_Creature_Cow.SM_Creature_Cow"); break;
        case EMCRCreatureType::Chicken: MeshPath=TEXT("/Game/Models/SM_Creature_Chicken.SM_Creature_Chicken"); break;
        case EMCRCreatureType::Boar: MeshPath=TEXT("/Game/Models/SM_Creature_Boar.SM_Creature_Boar"); break;
        default: MeshPath=TEXT("/Game/Models/SM_Creature_Sheep.SM_Creature_Sheep"); break;
    }
    if (UStaticMesh* ImportedCreature=LoadObject<UStaticMesh>(nullptr,*MeshPath))
    {
        Body->SetStaticMesh(ImportedCreature);
        Body->SetRelativeLocation(FVector::ZeroVector);
        Body->SetRelativeScale3D(FVector(.85f));
        if (Head) Head->SetVisibility(false,true);
        for (UStaticMeshComponent* Leg : Legs) if (Leg) Leg->SetVisibility(false,true);
    }
    if (UMaterialInterface* TintMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Creatures/Materials/M_CreatureTint.M_CreatureTint")))
    {
        Body->SetMaterial(0,TintMaterial);
        UMaterialInstanceDynamic* Mat=Body->CreateAndSetMaterialInstanceDynamic(0);
        if (Mat) Mat->SetVectorParameterValue(TEXT("Tint"),BodyColor);
        if (Head) { Head->SetMaterial(0,TintMaterial); Head->SetVectorParameterValueOnMaterials(TEXT("Tint"),FVector(BodyColor.R,BodyColor.G,BodyColor.B)); }
        for (UStaticMeshComponent* Leg : Legs) if (Leg) { Leg->SetMaterial(0,TintMaterial); Leg->SetVectorParameterValueOnMaterials(TEXT("Tint"),FVector(BodyColor.R,BodyColor.G,BodyColor.B)); }
    }
    WalkSpeed=Type==EMCRCreatureType::Chicken?125.0f:Type==EMCRCreatureType::Boar?105.0f:82.0f;
    SetActorScale3D(Type==EMCRCreatureType::Chicken?FVector(.58f):Type==EMCRCreatureType::Cow?FVector(1.25f):FVector(1.0f));
}
void AMCRCreatureActor::ChooseDirection()
{
    const float Angle=FMath::FRandRange(-PI,PI); MoveDirection=FVector(FMath::Cos(Angle),FMath::Sin(Angle),0.0f); DirectionTimer=FMath::FRandRange(2.5f,6.0f);
}
void AMCRCreatureActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds); DirectionTimer-=DeltaSeconds; AnimationTime+=DeltaSeconds; VocalTimer-=DeltaSeconds;
    if (VocalTimer<=0.0f)
    {
        if (USoundBase* Voice=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/SFX_Animal_Soft.SFX_Animal_Soft"))) UGameplayStatics::PlaySoundAtLocation(this,Voice,GetActorLocation(),0.32f);
        VocalTimer=FMath::FRandRange(9.0f,22.0f);
    }
    if (DirectionTimer<=0.0f) ChooseDirection();
    if (APawn* Player=UGameplayStatics::GetPlayerPawn(this,0))
    {
        const FVector Away=GetActorLocation()-Player->GetActorLocation();
        if (Away.Size2D()<360.0f && CreatureType!=EMCRCreatureType::Boar) MoveDirection=Away.GetSafeNormal2D();
    }
    const FVector Delta=MoveDirection*WalkSpeed*DeltaSeconds;
    FHitResult Hit; AddActorWorldOffset(Delta,true,&Hit);
    if (Hit.IsValidBlockingHit()) ChooseDirection();
    if (!MoveDirection.IsNearlyZero()) SetActorRotation(FMath::RInterpTo(GetActorRotation(),MoveDirection.Rotation(),DeltaSeconds,3.0f));
    for (int32 I=0; I<Legs.Num(); ++I)
    {
        if (Legs[I]) Legs[I]->SetRelativeRotation(FRotator(FMath::Sin(AnimationTime*8.0f+(I%2)*PI)*13.0f,0,0));
    }
}

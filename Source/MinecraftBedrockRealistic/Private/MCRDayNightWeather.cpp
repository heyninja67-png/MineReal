#include "MCRDayNightWeather.h"
#include "Components/SceneComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AMCRDayNightWeather::AMCRDayNightWeather()
{
    PrimaryActorTick.bCanEverTick = true;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root")); SetRootComponent(SceneRoot);
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun")); SunLight->SetupAttachment(SceneRoot); SunLight->SetMobility(EComponentMobility::Movable); SunLight->SetIntensity(8.0f);
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight")); SkyLight->SetupAttachment(SceneRoot); SkyLight->SetMobility(EComponentMobility::Movable); SkyLight->bRealTimeCapture = false; SkyLight->SetIntensity(0.8f);
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphereFog")); HeightFog->SetupAttachment(SceneRoot); HeightFog->SetFogDensity(0.012f); HeightFog->SetFogHeightFalloff(0.2f);
}
void AMCRDayNightWeather::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float SafeDayLength = FMath::Max(1.0f, DayLengthMinutes);
    TimeOfDayHours = FMath::Fmod(TimeOfDayHours + DeltaSeconds * 24.0f / (SafeDayLength * 60.0f), 24.0f);
    WeatherClock += DeltaSeconds;
    if (WeatherClock > 600.0f)
    {
        WeatherClock = 0.0f;
        const int32 Roll = FMath::RandRange(0, 99);
        SetWeather(Roll < 58 ? EMCRWeather::Clear : Roll < 78 ? EMCRWeather::Cloudy : Roll < 96 ? EMCRWeather::Rain : EMCRWeather::Storm);
    }
    UpdateLighting();
}
void AMCRDayNightWeather::SetTimeOfDay(float Hours) { TimeOfDayHours = FMath::Fmod(FMath::Max(0.0f, Hours), 24.0f); UpdateLighting(); }
void AMCRDayNightWeather::SetWeather(EMCRWeather Weather)
{
    CurrentWeather = Weather;
    if (HeightFog)
    {
        const float Density = Weather == EMCRWeather::Clear ? 0.008f : Weather == EMCRWeather::Cloudy ? 0.014f : Weather == EMCRWeather::Rain ? 0.022f : 0.032f;
        HeightFog->SetFogDensity(Density);
        HeightFog->SetFogInscatteringColor(Weather == EMCRWeather::Storm ? FLinearColor(.18f,.21f,.28f) : FLinearColor(.48f,.55f,.63f));
    }
    UpdateLighting();
}
void AMCRDayNightWeather::UpdateLighting()
{
    const float Angle = (TimeOfDayHours / 24.0f) * 360.0f - 90.0f;
    if (SunLight)
    {
        SunLight->SetWorldRotation(FRotator(Angle, -32.0f, 0.0f));
        const float Elevation = FMath::Sin(FMath::DegreesToRadians(Angle));
        const float DayFactor = FMath::Clamp((Elevation + 0.14f) / 0.8f, 0.025f, 1.0f);
        const float WeatherFactor = CurrentWeather == EMCRWeather::Storm ? 0.24f : CurrentWeather == EMCRWeather::Rain ? 0.48f : CurrentWeather == EMCRWeather::Cloudy ? 0.72f : 1.0f;
        SunLight->SetIntensity(8.0f * DayFactor * WeatherFactor);
        SunLight->SetLightColor(FLinearColor::LerpUsingHSV(FLinearColor(1.0f,0.48f,0.25f), FLinearColor(1.0f,0.94f,0.82f), FMath::Clamp(Elevation+0.2f,0.0f,1.0f)));
    }
    if (SkyLight) SkyLight->SetIntensity(IsNight() ? 0.16f : (CurrentWeather == EMCRWeather::Storm ? 0.35f : 0.8f));
}

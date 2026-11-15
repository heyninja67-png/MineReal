#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCRDayNightWeather.generated.h"
class UDirectionalLightComponent; class USkyLightComponent; class UExponentialHeightFogComponent; class USceneComponent;
UENUM(BlueprintType) enum class EMCRWeather : uint8 { Clear, Cloudy, Rain, Storm };
UCLASS()
class MINECRAFTBEDROCKREALISTIC_API AMCRDayNightWeather : public AActor
{
    GENERATED_BODY()
public:
    AMCRDayNightWeather();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Time") float DayLengthMinutes = 24.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Time", meta=(ClampMin="0", ClampMax="24")) float TimeOfDayHours = 9.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weather") EMCRWeather CurrentWeather = EMCRWeather::Clear;
    UFUNCTION(BlueprintCallable, Category="Time") void SetTimeOfDay(float Hours);
    UFUNCTION(BlueprintCallable, Category="Weather") void SetWeather(EMCRWeather Weather);
    UFUNCTION(BlueprintPure, Category="Time") bool IsNight() const { return TimeOfDayHours < 6.0f || TimeOfDayHours > 19.0f; }
private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UDirectionalLightComponent> SunLight;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USkyLightComponent> SkyLight;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UExponentialHeightFogComponent> HeightFog;
    float WeatherClock = 0.0f;
    void UpdateLighting();
};

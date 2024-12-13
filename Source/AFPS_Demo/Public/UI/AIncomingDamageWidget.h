// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIncomingDamageWidget.generated.h"

class UCanvasPanelSlot;

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API UAIncomingDamageWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="UI")
	float Radius = 250.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (ExposeOnSpawn=true))
	FVector DamageSourceLocation;

	UFUNCTION(BlueprintImplementableEvent)
	void SetRelativeAngle(const float InAngle);
};

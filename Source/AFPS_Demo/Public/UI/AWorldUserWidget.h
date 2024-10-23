#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AWorldUserWidget.generated.h"

class USizeBox;

/**
 * 
 */
UCLASS()
class AFPS_DEMO_API UAWorldUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (ExposeOnSpawn = true))
	AActor* AttachedActor;

	UPROPERTY(EditAnywhere, Category = "UI")
	FVector WorldOffset;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	USizeBox* ParentSizeBox;
};

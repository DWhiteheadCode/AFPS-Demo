#include "UI/AIncomingDamageWidget.h"

#include "Kismet/KismetMathLibrary.h"

void UAIncomingDamageWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APawn* OwningPawn = GetOwningPlayerPawn();

	if (!IsValid(OwningPawn))
	{
		RemoveFromParent();
		UE_LOG(LogTemp, Warning, TEXT("OwningPawn not valid. Removing IncomingDamageWidget."));
		return;
	}

	DrawDebugSphere(OwningPawn->GetWorld(), DamageSourceLocation, 5.f, 16, FColor::White, false, 0.f, 0, 1.f);
	
	const FRotator RelativeRotationToDamageSource = UKismetMathLibrary::FindRelativeLookAtRotation(OwningPawn->GetActorTransform(), DamageSourceLocation);
	
	// Convert angle to 0-360. Only interested in yaw.
	float AngleToDamageSource = RelativeRotationToDamageSource.Yaw;
	if (AngleToDamageSource < 0)
	{
		AngleToDamageSource += 360.f;
	}

	UE_LOG(LogTemp, Log, TEXT("Angle: [%f]"), AngleToDamageSource);

	// Only interested in Yaw.
	SetRelativeAngle(AngleToDamageSource); 
}
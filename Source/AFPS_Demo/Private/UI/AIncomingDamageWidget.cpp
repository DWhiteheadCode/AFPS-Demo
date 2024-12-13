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
	
	const FRotator RelativeRotationToDamageSource = UKismetMathLibrary::FindRelativeLookAtRotation(OwningPawn->GetActorTransform(), DamageSourceLocation);
	
	// Convert angle to 0-360. Only interested in yaw.
	float AngleToDamageSource = RelativeRotationToDamageSource.Yaw;
	if (AngleToDamageSource < 0)
	{
		AngleToDamageSource += 360.f;
	}

	// Only interested in Yaw.
	SetRelativeAngle(AngleToDamageSource); 
}
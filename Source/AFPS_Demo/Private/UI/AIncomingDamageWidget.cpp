#include "UI/AIncomingDamageWidget.h"

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
}
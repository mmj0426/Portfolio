#include "Portfolio/PathFind/NavCheckPoint.h"

#if WITH_EDITOR
#include "Editor.h"
#include "FileHelpers.h"
#endif

ANavCheckPoint::ANavCheckPoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ANavCheckPoint::BeginPlay()
{
	Super::BeginPlay();
	CheckPointData.Location = GetActorLocation();
}
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

//void ANavCheckPoint::SetDirtyExternalActor()
//{
//#if WITH_EDITOR
//	UPackage* Package = GetPackage();
//	if (Package)
//	{
//		Package->SetFlags(RF_Transactional);
//		Package->Modify();
//		Package->MarkPackageDirty();
//	}
//#endif
//}
//
//void ANavCheckPoint::SaveExternalActor()
//{
//#if WITH_EDITOR
//	UPackage* Package = GetPackage();
//	if (Package)
//	{
//		Package->SetFlags(RF_Transactional);
//		Package->Modify();
//		Package->MarkPackageDirty();
//		FEditorFileUtils::PromptForCheckoutAndSave({ Package }, true, false);
//	}
//#endif
//}

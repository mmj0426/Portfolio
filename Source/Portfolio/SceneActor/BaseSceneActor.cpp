#include "SceneActor/BaseSceneActor.h"
#include "Components/SceneComponent.h"

#include "PortfolioCharacter.h"
#include "Camera/CameraComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameDelegates.h"

ABaseSceneActor* ABaseSceneActor::CurrentActiveSceneActor = nullptr;

void ABaseSceneActor::SetActiveSceneActor(ABaseSceneActor* ActiveSceneActor)
{
	if (CurrentActiveSceneActor == ActiveSceneActor)
	{
		return;
	}

	if (IsValid(CurrentActiveSceneActor))
	{
		CurrentActiveSceneActor->Deactivate();
	}

	CurrentActiveSceneActor = ActiveSceneActor;

	if (IsValid(CurrentActiveSceneActor))
	{
		CurrentActiveSceneActor->Activate();
	}
}

ABaseSceneActor::ABaseSceneActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultScene = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultScene"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	LightComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("LightComp"));

	SetRootComponent(DefaultScene);
	CameraComp->SetupAttachment(DefaultScene);
	LightComponent->SetupAttachment(DefaultScene);
}

void ABaseSceneActor::BeginPlay()
{
	Super::BeginPlay();

	InitFOV = CameraComp->FieldOfView;

	PrevActiveSceneActor = ABaseSceneActor::CurrentActiveSceneActor;

	// 이전에 생성된 SceneActor 없이 처음 생성되었을 때
	if (PrevActiveSceneActor == nullptr)
	{
		if (IsValid(GetWorld()) && (InteractionData.AxisHandlingModeX != EAxisHandlingMode::None || InteractionData.AxisHandlingModeY != EAxisHandlingMode::None || InteractionData.bEnableZoom))
		{
			InputInteractor = NewObject<UInputInteractor>(this);
			InputInteractor->InitializeInteractionData(InteractionData);
		}

		// 인게임의 Light 영향을 받지 않도록 인게임 Light를 비활성화 합니다.
	}
	SetActiveSceneActor(this);
}

void ABaseSceneActor::InitInputInteractor_Implementation()
{
	UWorld* World = GetWorld();
	if (false == IsValid(World)) { return; }

	if (InputInteractor)
	{
		// 입력 시스템 클래스에서 바인딩합니다.
		// 1. 움직이는 입력 시 (OnInputMoved) -> OnScreenDrag
		// 2. Zoom In/Out 입력 시 (OnInputZoomed) -> OnScreenZoom
	}
}

void ABaseSceneActor::SetCameraFOV_Implementation(float Value)
{
	UWorld* World = GetWorld();

	if (nullptr == World)
	{
		return;
	}

	if (APlayerController* Controller = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (APortfolioCharacter* Character = Cast<APortfolioCharacter>(Controller->GetCharacter()))
		{
			if (UCameraComponent* Camera = Cast<UCameraComponent>(Character->GetFollowCamera()))
			{
				if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0))
				{
					PlayerCameraFOV = CameraManager->GetFOVAngle();
					CameraManager->SetFOV(Value);
				}
			}
		}
		Controller->SetViewTarget(this);
	}
}

void ABaseSceneActor::ResetCameraToPlayerView()
{
	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		return;
	}

	if (APlayerController* Controller = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (APortfolioCharacter* Character = Cast<APortfolioCharacter>(Controller->GetCharacter()))
		{
			if (UCameraComponent* Camera = Cast<UCameraComponent>(Character->GetFollowCamera()))
			{
				if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(World, 0))
				{
					PlayerCameraFOV = CameraManager->GetFOVAngle();
					CameraManager->SetFOV(PlayerCameraFOV);
				}
			}
		}
		Controller->SetViewTarget(UGameplayStatics::GetPlayerCharacter(World, 0));
	}
}

void ABaseSceneActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(PrevActiveSceneActor))
	{
		ABaseSceneActor::SetActiveSceneActor(PrevActiveSceneActor);
	}
	else
	{
		SetSubTalkVisibility(true);
		ABaseSceneActor::SetActiveSceneActor(nullptr);
		ResetCameraToPlayerView();

		// 인게임의 Light를 활성화 합니다.
	}

	if (InputInteractor)
	{
		InputInteractor->StopInputInteraction();
		InputInteractor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ABaseSceneActor::Activate()
{
	SetSubTalkVisibility(false);
	SetCameraFOV(InitFOV);
	InitInputInteractor();
	SetActorHiddenInGame(false);

	ApplySceneActorBGLight(CurrentActiveSceneActor->LitPresetID);
}

void ABaseSceneActor::Deactivate()
{
	UWorld* World = GetWorld();
	if (false == IsValid(World)) { return; }

	SetActorHiddenInGame(true);

	// Unbind Input
	if (InputInteractor)
	{
		// 입력 시스템 클래스에서 바인딩했던 이벤트를 해제합니다.
	}

	ApplySceneActorBGLight(NAME_None);
}

void ABaseSceneActor::OnScreenDrag(FVector Location, FVector2D DeltaValue)
{
	if (InputInteractor)
	{
		InputInteractor->OnInputMoved(DeltaValue);
	}
}

void ABaseSceneActor::OnScreenZoom(float DeltaValue)
{
	if (InputInteractor)
	{
		InputInteractor->OnInputWheelEvent(DeltaValue);
	}
}

void ABaseSceneActor::SetSubTalkVisibility(bool CanShow)
{
	// SubTalk (인게임 대화)의 Visibility를 설정합니다
}

USceneComponent* ABaseSceneActor::GetZoomControlTarget_Implementation() const
{
	return CameraComp;
}

USceneComponent* ABaseSceneActor::GetTranslationTarget_Implementation() const
{
	return CameraComp;
}

float ABaseSceneActor::GetCameraFOV_Implementation() const
{
	return CameraComp->FieldOfView;
}

#pragma region Light
void ABaseSceneActor::ApplySceneActorBGLight(FName ID)
{
	// SceneActor의 라이트를 테이블에 등록된 LitPreset을 등록합니다.
}
#pragma endregion



#include "VisualEffect/StatusVisualEffectComponent.h"

#pragma region Character
#include "PortfolioCharacter.h"
#include "VisualEffect/StatusVisualEffect.h"
#include "Camera/CameraComponent.h"
#pragma endregion

#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UStatusVisualEffectComponent::UStatusVisualEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatusVisualEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickInterval(.1f);
	SetComponentTickEnabled(true);

	OwnerCharacter = Cast<APortfolioCharacter>(GetOwner());

	if (false == OwnerCharacter.IsValid())
	{
		DestroyComponent();
		return;
	}

	FSimpleDelegate EnterGateFunction;
	EnterGateFunction.BindUObject(this, &ThisClass::OnRotateIconGate);
	RotateIconGate = FFCGate(EnterGateFunction);

	if (!IconEffectSceneComp)
	{
		IconEffectSceneComp = Cast<USceneComponent>(OwnerCharacter->AddComponentByClass(USceneComponent::StaticClass(), false, FTransform::Identity, false));
	}

	PlayerCharacterCameraComp = OwnerCharacter->GetFollowCamera();

	OwnerCharacter->OnCharacterDied.AddDynamic(this, &ThisClass::OnOwnCharacterDie);
	LastCharacterRotation = OwnerCharacter->GetActorRotation();
}

void UStatusVisualEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckRotateIconGate();
	RotateIconGate.Enter();
}

void UStatusVisualEffectComponent::CheckRotateIconGate()
{
	FRotator CurOwnerRotation = OwnerCharacter->GetActorRotation();
	FVector CurCameraLocation = PlayerCharacterCameraComp->GetRelativeLocation();

	if (LastCharacterRotation == CurOwnerRotation && LastCameraLocation == CurCameraLocation)
	{
		CloseRotateIconGate();
	}
	else
	{
		LastCharacterRotation = CurOwnerRotation;
		LastCameraLocation = CurCameraLocation;
		OpenRotateIconGate();
	}
}

void UStatusVisualEffectComponent::OnRotateIconGate()
{
	IconEffectSceneComp->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(IconEffectSceneComp->GetComponentLocation(), PlayerCharacterCameraComp->GetRelativeLocation()));
}

void UStatusVisualEffectComponent::OpenRotateIconGate()
{
	if (IconSequence.IsEmpty())
	{
		return;
	}

	if (PlayerCharacterCameraComp.IsValid() && IconEffectSceneComp)
	{
		RotateIconGate.Open();
	}
}

void UStatusVisualEffectComponent::CloseRotateIconGate()
{
	RotateIconGate.Close();
}

void UStatusVisualEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CloseRotateIconGate();
	DestroyAllEffect();

	Super::EndPlay(EndPlayReason);
}

void UStatusVisualEffectComponent::SpawnEffectActor(skill_condition_terms_id skillConditionType)
{
	// 캐릭터 사망 여부를 판단하여 사망 상태일 경우 반환합니다.

	TSubclassOf<AStatusVisualEffect> effectObjectClass = GetDebuffEffectResource(skillConditionType);

	if (!effectObjectClass || !IconEffectSceneComp)
	{
		return;
	}

	IconEffectSceneComp->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(IconEffectSceneComp->GetComponentLocation(), PlayerCharacterCameraComp->GetRelativeLocation()));

	UWorld* World = GetWorld();
	if (OwnerCharacter.IsValid() && World)
	{
		if (TObjectPtr <AStatusVisualEffect> effectActor = World->SpawnActor<AStatusVisualEffect>(effectObjectClass, FTransform::Identity))
		{
			effectActor->OnSpawnedIconEffect.BindUObject(this, &ThisClass::OnSpawnedNewIcon);
			effectActor->AttachToActor(OwnerCharacter.Get(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false), NAME_None);
			SpawnedDebuffEffectActor.Emplace(skillConditionType, effectActor);

			effectActor->InitEffectActor(OwnerCharacter, IconEffectSceneComp);
			effectActor->ActiveDebuffEffect(skillConditionType);
		}
	}
}

TSubclassOf<AStatusVisualEffect> UStatusVisualEffectComponent::GetDebuffEffectResource(skill_condition_terms_id type) const
{
	// skill_condition_terms_id type에 따른 이펙트 리소스 데이터를 가져옵니다.
	return TSubclassOf<AStatusVisualEffect>();
}

void UStatusVisualEffectComponent::OnSpawnedNewIcon(skill_condition_terms_id debuffType, UNiagaraComponent* loadedNiagaraComp)
{
	if (!loadedNiagaraComp || debuffType == skill_condition_terms_id::SCTI_ZERO)
	{
		return;
	}

	IconSequence.Emplace(debuffType);
	SpawnedIconEffects.Emplace(debuffType, loadedNiagaraComp);

	RelocateIconEffect();
}

void UStatusVisualEffectComponent::RelocateIconEffect()
{
	if (!IconEffectSceneComp)
	{
		return;
	}

	double startLocation = -1 * (SpawnedIconEffects.Num() - 1) * IconGap / 2;
	int32 lastIndex = SpawnedIconEffects.Num();
	if (lastIndex <= 0)
	{
		return;
	}

	int32 index = 0;
	for (skill_condition_terms_id debuffType : IconSequence)
	{
		if (index > lastIndex)
		{
			return;
		}

		if (SpawnedIconEffects.Contains(debuffType))
		{
			if (UNiagaraComponent* niagaraComp = SpawnedIconEffects.FindRef(debuffType))
			{
				if (niagaraComp)
				{
					double length = (startLocation + (index * IconGap));

					// SceneComponent는 카메라를 바라보기 때문에 RightVecot는 항상 왼쪽이 + 이므로 *-1
					niagaraComp->SetWorldLocation(IconEffectSceneComp->GetComponentLocation() + (length * -1.f * IconEffectSceneComp->GetRightVector()));
					index++;
				}
			}
		}
	}
}

TSoftObjectPtr<UTexture> UStatusVisualEffectComponent::GetEffectDiffuseTexture(skill_condition_terms_id skillConditionType)const
{
	if (SpawnedDebuffEffectActor.Contains(skillConditionType))
	{
		if (TObjectPtr<AStatusVisualEffect> spawnedEffect = SpawnedDebuffEffectActor.FindRef(skillConditionType))
		{
			return spawnedEffect->GetEffectDiffuseTexture();
		}
	}

	return nullptr;
}

void UStatusVisualEffectComponent::DestroyEffectActor(skill_condition_terms_id skillConditionType)
{
	if (SpawnedDebuffEffectActor.Contains(skillConditionType))
	{
		if (TObjectPtr<AStatusVisualEffect> effectActor = SpawnedDebuffEffectActor.FindRef(skillConditionType))
		{
			if (IsValid(effectActor))
			{
				effectActor->Destroy();
			}			
		}
	}
		
	IconSequence.Remove(skillConditionType);
	SpawnedIconEffects.Remove(skillConditionType);
	SpawnedDebuffEffectActor.Remove(skillConditionType);

	RelocateIconEffect();
}

void UStatusVisualEffectComponent::DestroyAllEffect()
{
	for (TPair<skill_condition_terms_id, AStatusVisualEffect*> EffectActor : SpawnedDebuffEffectActor)
	{
		if (EffectActor.Value)
		{
			EffectActor.Value->Destroy();
		}
	}

	IconSequence.Empty();
	SpawnedIconEffects.Empty();
	SpawnedDebuffEffectActor.Empty();
}

void UStatusVisualEffectComponent::OnOwnCharacterDie(APortfolioCharacter* Character)
{
	DestroyAllEffect();
}

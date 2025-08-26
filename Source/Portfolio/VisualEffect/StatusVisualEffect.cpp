#include "StatusVisualEffect.h"
#include "PortfolioCharacter.h"

#pragma region Niagara
#include "NiagaraCommon.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#pragma endregion

AStatusVisualEffect::AStatusVisualEffect()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStatusVisualEffect::InitEffectActor(TWeakObjectPtr<class APortfolioCharacter> Character, TWeakObjectPtr<class USceneComponent> iconSceneComp)
{
	OwnerCharacter = Character;
	IconEffectSceneComp = iconSceneComp;

	if (OwnerCharacter.IsValid())
	{
		OwnerCharacter->OnCharacterDied.AddDynamic(this, &ThisClass::OnOwnCharacterDie);

		if (IconEffectSceneComp.IsValid() && IsValidIconEffect())
		{
			IconEffectSceneComp->SetRelativeLocation(GetSpawnLocationByType(IconEffect.SpawnLocationType) + IconEffect.GapFromCapsule);
		}
	}
}

void AStatusVisualEffect::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false);
}

void AStatusVisualEffect::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerCharacter.IsValid())
	{
		if (IsAnimationStop)
		{
			OwnerCharacter->GetMesh()->GlobalAnimRateScale = GlobalAnimRateBeforeStop;
		}

		if (CurrentAnimMontage.IsValid())
		{
			if (DebuffType == skill_condition_terms_id::SCTI_KNOCKDOWN)
			{
				OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("Knockdown_End"), CurrentAnimMontage.Get());
			}
			else
			{
				OwnerCharacter->StopAnimMontage(CurrentAnimMontage.Get());
			}
		}

		ApplyDebuffEffectHUD(false);
	}

	if (IconNiagaraComp.IsValid())
	{
		IconNiagaraComp->DestroyComponent();
	}

	if (DebuffNiagaraComp.IsValid())
	{
		DebuffNiagaraComp->DestroyComponent();
	}

	Super::EndPlay(EndPlayReason);
}

void AStatusVisualEffect::OnOwnCharacterDie(APortfolioCharacter* Character)
{
	Destroy();
}

const FVector AStatusVisualEffect::GetSpawnLocationByType(EStatusVisualEffectSpawnLocType spawnLocType) const
{
	switch (spawnLocType)
	{
		case EStatusVisualEffectSpawnLocType::ESpawnLoc_None: return FVector::ZeroVector;
		case EStatusVisualEffectSpawnLocType::ESpawnLoc_Bottom: return FVector(0.f, 0.f, -1 * OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		case EStatusVisualEffectSpawnLocType::ESpawnLoc_Center: return FVector::ZeroVector;
		case EStatusVisualEffectSpawnLocType::ESpawnLoc_Top: return FVector(0.f, 0.f, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		default:return FVector::ZeroVector;
	}
}

void AStatusVisualEffect::OnLoadedNiagaraAsset(TObjectPtr<UObject> NiagaraAsset, FStatusVisualEffectData Data, bool isIcon)
{
	// ĳ���� ��� �� ��ȯ
	//if (false == OwnerCharacter->IsAlive())
	//{
	//	return;
	//}

	UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(NiagaraAsset);

	if (false == IsValid(NiagaraSystem) || false == IconEffectSceneComp.IsValid())
	{
		return;
	}

	USceneComponent* attachComponent = isIcon ? IconEffectSceneComp.Get() : GetRootComponent();

	// �������� ��� SceneComponent�� ��ġ�� ��ұ� ����
	FVector SpawnLocation = isIcon ? FVector::ZeroVector : GetSpawnLocationByType(Data.SpawnLocationType) + Data.GapFromCapsule;

	TObjectPtr<UNiagaraComponent> NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, attachComponent, NAME_None, SpawnLocation, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, true, ENCPoolMethod::None, true);

	if (isIcon)
	{
		IconNiagaraComp = NiagaraComp;
		OnSpawnedIconEffect.ExecuteIfBound(static_cast<skill_condition_terms_id>(DebuffType), IconNiagaraComp.Get());
	}
	else
	{
		DebuffNiagaraComp = NiagaraComp;
	}
}

void AStatusVisualEffect::ActiveDebuffEffect(skill_condition_terms_id ActiveConditionType)
{
	if (DebuffType != ActiveConditionType)
	{
		UE_LOG(LogTemp, Log, TEXT("Current Character State Type is different from Resource State Type !!"));
	}

	// Icon Effect Niagara�� �񵿱� �ε��ϰ� �Ϸ� �� OnLoadedNiagaraAsset() �Լ��� ���ε��մϴ�.
	// Debuff Effect Niagara �񵿱� �ε��ϰ� �Ϸ� �� OnLoadedNiagaraAsset() �Լ��� ���ε��մϴ�.

	if (IsAnimationStop)
	{
		StopAnimation();
	}

	// ��Ÿ�� �÷���

	ApplyDebuffEffectHUD(true);
}

void AStatusVisualEffect::ApplyDebuffEffectHUD(bool IsActive)
{
	// �����̻� HUD ���� Open / Close
}

void AStatusVisualEffect::StopAnimation_Implementation()
{
	if (false == OwnerCharacter.IsValid())
	{
		return;
	}

	GlobalAnimRateBeforeStop = OwnerCharacter->GetMesh()->GlobalAnimRateScale;
	OwnerCharacter->GetMesh()->GlobalAnimRateScale = 0.f;
}

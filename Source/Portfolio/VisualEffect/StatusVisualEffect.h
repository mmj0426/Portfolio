#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

//#include "GameplayTagContainer.h"

#include "NiagaraDataInterfaceExport.h"
#include "VisualEffect/SkillConditionType.h"
#include "StatusVisualEffect.generated.h"

enum class ECharacterAnimMontageType : uint8;

// 이펙트 출력 위치
UENUM(BlueprintType)
enum class EStatusVisualEffectSpawnLocType : uint8
{
	ESpawnLoc_None				UMETA(Hidden),
	ESpawnLoc_Bottom			UMETA(DisplayName = "CharacterBottom"),		// 캐릭터 발 끝
	ESpawnLoc_Center			UMETA(DisplayName = "CharacterCenter"),		// 캐릭터  중앙
	ESpawnLoc_Top				UMETA(DisplayName = "CharacterTop"),		// 캐릭터 머리 위
};

USTRUCT(BlueprintType)
struct FStatusVisualEffectData
{
	GENERATED_USTRUCT_BODY()

	// 출력할 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class UNiagaraSystem> NiagaraEffect;

	// 출력할 위치 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStatusVisualEffectSpawnLocType SpawnLocationType = EStatusVisualEffectSpawnLocType::ESpawnLoc_None;

	// 출력할 위치로부터 떨어진 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector  GapFromCapsule = FVector::ZeroVector;
};

DECLARE_DELEGATE_TwoParams(FOnSpawnedIconEffect, skill_condition_terms_id, UNiagaraComponent*)

UCLASS(BlueprintType, Blueprintable)
class PORTFOLIO_API AStatusVisualEffect : public AActor, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
	
public:	
	AStatusVisualEffect();
	FOnSpawnedIconEffect OnSpawnedIconEffect;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void InitEffectActor(TWeakObjectPtr<class APortfolioCharacter> Character, TWeakObjectPtr<class USceneComponent> iconSceneComp);

public:	
	FORCEINLINE bool IsValidIconEffect()const { return !(IconEffect.NiagaraEffect.IsNull()); }

	const FVector GetSpawnLocationByType(EStatusVisualEffectSpawnLocType spawnLocType) const;

	// Niagara 비동기 로드 완료 시 바인딩
	void OnLoadedNiagaraAsset(TObjectPtr<UObject> NiagaraAsset, FStatusVisualEffectData Data, bool isIcon);

	void ActiveDebuffEffect(skill_condition_terms_id ActiveConditionType);
	void ApplyDebuffEffectHUD(bool IsActive);

	UFUNCTION(BlueprintNativeEvent)
	void StopAnimation();
	void StopAnimation_Implementation();

	FORCEINLINE TSoftObjectPtr<UTexture> GetEffectDiffuseTexture()const { return EffectDiffuseTexture; }

protected:
	UFUNCTION() void OnOwnCharacterDie(class APortfolioCharacter* Character);

	UPROPERTY() TWeakObjectPtr<class APortfolioCharacter> OwnerCharacter;
	UPROPERTY() TWeakObjectPtr<USceneComponent> IconEffectSceneComp;
	
	UPROPERTY() TWeakObjectPtr<UNiagaraComponent>IconNiagaraComp;
	UPROPERTY() TWeakObjectPtr<UNiagaraComponent> DebuffNiagaraComp;
	UPROPERTY() TWeakObjectPtr<UAnimMontage> CurrentAnimMontage;

	float GlobalAnimRateBeforeStop = 0.f;

	// 상태이상 종류
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	TEnumAsByte<skill_condition_terms_id> DebuffType = skill_condition_terms_id::SCTI_ZERO;

	// 상태이상 아이콘 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	FStatusVisualEffectData IconEffect;
	
	// 상태이상 이펙트 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	FStatusVisualEffectData DebuffEffect;

	// 애니메이션 정지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	bool IsAnimationStop = false;

	// 상태이상 플레이 할 몽타주 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	uint8/*ECharacterAnimMontageType*/ PlayMontageType = 0;

	// 변경할 Diffuse Texture
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	TSoftObjectPtr<UTexture> EffectDiffuseTexture;

	// 상태이상 HUD Type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	uint8/*EEventWidgetType*/ HUDWidgetType = 0;
};

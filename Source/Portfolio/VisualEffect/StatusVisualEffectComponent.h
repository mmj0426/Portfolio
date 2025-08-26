#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/FlowControl.h"
#include "VisualEffect/SkillConditionType.h"
#include "StatusVisualEffectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UStatusVisualEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStatusVisualEffectComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// 중첩 상태일 때 아이콘과 아이콘 사이의 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	float IconGap = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TWeakObjectPtr<class APortfolioCharacter> OwnerCharacter;

	TWeakObjectPtr<class UCameraComponent> PlayerCharacterCameraComp;

	UPROPERTY() TObjectPtr<USceneComponent> IconEffectSceneComp;

	TMap<skill_condition_terms_id, class AStatusVisualEffect*> SpawnedDebuffEffectActor;
	TMap<skill_condition_terms_id, UNiagaraComponent*> SpawnedIconEffects;
	TArray<skill_condition_terms_id> IconSequence;

	FRotator LastCharacterRotation = FRotator::ZeroRotator;
	FVector LastCameraLocation = FVector::ZeroVector;

	UPROPERTY() 
	FFCGate RotateIconGate;

	void OnRotateIconGate();
	void OpenRotateIconGate();
	void CloseRotateIconGate();
	void CheckRotateIconGate();

public :  
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SpawnEffectActor(skill_condition_terms_id skillConditionType);
	void RelocateIconEffect();
	void DestroyEffectActor(skill_condition_terms_id skillConditionType);
	void DestroyAllEffect();

	UFUNCTION() void OnOwnCharacterDie(class APortfolioCharacter* Character);
	UFUNCTION() void OnSpawnedNewIcon(skill_condition_terms_id debuffType, UNiagaraComponent* loadedNiagaraComp);

	TSoftObjectPtr<UTexture> GetEffectDiffuseTexture(skill_condition_terms_id skillConditionType) const;
	TSubclassOf<class AStatusVisualEffect> GetDebuffEffectResource(skill_condition_terms_id type) const;
};

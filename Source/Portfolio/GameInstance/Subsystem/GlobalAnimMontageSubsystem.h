#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VisualEffect/SkillConditionType.h"
#include "GlobalAnimMontageSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FGlobalAnimMontageDescriptorBase : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * 이펙트의 고유 키를 반환합니다.
	 *
	 * @return 기본적으로 0을 반환합니다. 파생 구조체에서는 이 값을 오버라이드할 수 있습니다.
	 */
	FORCEINLINE virtual uint64 GetKey() const { return 0; }

	FORCEINLINE const FString& GetAssetPath() { return AssetPath; }

	/**
	 * 이 메서드는 서브클래스에서 오버라이드될 수 있으며, 소유한 데이터 테이블이 수정될 때마다 모든 행에 대해 호출됩니다.
	 * 사용자 변경사항에 대한 커스텀 수정, 파싱 등을 허용합니다.
	 * 이는 데이터를 가져올 때 OnPostDataImport와 함께 호출됩니다.
	 * 
	 * @param InDataTable					이 행을 소유하는 데이터 테이블입니다.
	 * @param InRowName						수정을 수행 중인 행의 이름입니다.
	 */
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;

protected:
	UPROPERTY(VisibleAnywhere) FString AssetPath;

	// 몽타주 경로
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UAnimMontage> MontagePath;

#if WITH_EDITORONLY_DATA
	// 개발자용 주석입니다. 에디터에서만 사용됩니다.
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FString DevComment;
#endif
};

USTRUCT(BlueprintType)
struct FStatusAnimMontageDescriptor : public FGlobalAnimMontageDescriptorBase
{
	GENERATED_BODY()

	FORCEINLINE virtual uint64 GetKey() const override { return FStatusAnimMontageDescriptor::GenerateKey(DebuffType, Tribe, Gender, WeaponType);}

	FORCEINLINE static uint64 GenerateKey(TEnumAsByte<skill_condition_terms_id> InDebuffType, uint8 InTribe, uint8 InGender, uint8/*TEnumAsByte<item_weapon_type>*/ InWeaponType)
	{
		return (static_cast<uint64>(InDebuffType) << 32) |
				(static_cast<uint64>(InTribe) << 16) |
				(static_cast<uint64>(InGender) << 8) |
				(static_cast<uint64>(InWeaponType));
	}

private :
	// 디버프 타입
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<skill_condition_terms_id> DebuffType = skill_condition_terms_id::SCTI_ZERO;

	// 종족
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	uint8 Tribe = 0;

	// 성별
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	uint8 Gender = 0;

	// 무기 타입
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	uint8/*TEnumAsByte<item_weapon_type>*/ WeaponType = 0;
};


UCLASS(Abstract, Blueprintable)
class PORTFOLIO_API UGlobalAnimMontageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public :
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

	UAnimMontage* GetMontageAsset(FName StructName, uint64 TableKey);

protected :

	// 테이블의 몽타주 에셋 경로를 캐싱해둠
	UFUNCTION() void LoadMontagePath();
	void AnimMontageDataTableLoadCompleted(const FSoftObjectPath& Path, const TSoftObjectPtr<UObject>& Table);

	UFUNCTION() void BindGameFlowEvent();
	UFUNCTION() void ClearCachedPaths();

	void Finalize();

protected :
	bool bIsInitialized = false;
	
	// 키 값에 따른 몽타주 에셋 경로
	// Key (TPair<FName, uint64>) : StructName과 고유 Key값
	// Value (FString) :  몽타주 애셋 경로
	TMap<TPair<FName, uint64>, FString> MontageAssetPaths;

	// 애님 몽타주 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FSoftObjectPath> AnimMontageTable;
};

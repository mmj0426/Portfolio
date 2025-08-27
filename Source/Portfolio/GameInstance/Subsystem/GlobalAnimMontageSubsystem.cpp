#include "GameInstance/Subsystem/GlobalAnimMontageSubsystem.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY_STATIC(LogGlobalAnimMontageSubsystem, All, All);

#pragma region FAnimMontageDescriptorBase

void FGlobalAnimMontageDescriptorBase::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	// 등록된 몽타주 소프트 오브젝트 패스를 문자열로 변환하여 저장합니다.
	AssetPath = MontagePath.ToString();
}

#pragma endregion


void UGlobalAnimMontageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BindGameFlowEvent();
}

void UGlobalAnimMontageSubsystem::Deinitialize()
{
	// 바인딩된 이벤트를 해제하고, 캐싱된 경로를 삭제합니다.

	ClearCachedPaths();
	Super::Deinitialize();
}

void UGlobalAnimMontageSubsystem::BindGameFlowEvent()
{
	// 게임 준비 단계에서 전역적으로 관리할 애니메이션 몽타주를 로드 및 초기화 시점을 위해 이벤트를 바인딩합니다.
	// 1. 애니메이션 몽타주 준비 시작 : Load Montage Path
	// 2. 게임 상태 초기화 시 : Clear Cached Path
}

void UGlobalAnimMontageSubsystem::ClearCachedPaths()
{
	bIsInitialized = false;
	MontageAssetPaths.Empty();
}

UAnimMontage* UGlobalAnimMontageSubsystem::GetMontageAsset(FName StructName, uint64 TableKey)
{
	TPair<FName, uint64> Key(StructName, TableKey);
	if (MontageAssetPaths.Contains(Key))
	{
		FString MontageAssetPath = MontageAssetPaths.FindRef(Key);
		if (false == MontageAssetPath.IsEmpty())
		{
			return Cast<UAnimMontage>(FSoftObjectPath(MontageAssetPath).TryLoad());
		}
	}
	return nullptr;
}

void UGlobalAnimMontageSubsystem::LoadMontagePath()
{
	for (FSoftObjectPath Path : AnimMontageTable)
	{
		UE_LOG(LogGlobalAnimMontageSubsystem, Log, TEXT("Path : %s"), *Path.ToString());
		TSoftObjectPtr<UDataTable> DataTableSoftPtr = TSoftObjectPtr<UDataTable>(TSoftObjectPtr<UDataTable>(Path));
		AnimMontageDataTableLoadCompleted(Path, DataTableSoftPtr.LoadSynchronous());
	}

	Finalize();
}

void UGlobalAnimMontageSubsystem::AnimMontageDataTableLoadCompleted(const FSoftObjectPath& Path, const TSoftObjectPtr<UObject>& Table)
{
	UDataTable* DataTable = Cast<UDataTable>(Table.Get());
	if (nullptr == DataTable)
	{
		UE_LOG(LogGlobalAnimMontageSubsystem, Warning, TEXT("Failed to load [%s] AnimMontageDataTable. Please check if the table exists and the path is correct, then try again."), *Path.ToString());
		return;
	}

	// 로드된 데이터 테이블이 FGlobalAnimMontageDescriptorBase 타입과 호환되는지 확인
	if (false == DataTable->GetRowStruct()->IsChildOf(FGlobalAnimMontageDescriptorBase::StaticStruct()))
	{
		UE_LOG(LogGlobalAnimMontageSubsystem, Warning, TEXT("Unsupported data table [%s] detected. Please verify the data table format is compatible with the current system requirements."), *DataTable->GetRowStruct()->GetName());
		return;
	}

	FName StructName = DataTable->RowStruct->GetFName();

	TArray<FGlobalAnimMontageDescriptorBase*> Rows;
	const FString& ContextString = DataTable->GetName() + TEXT("GetAllRows");
	DataTable->GetAllRows(ContextString, Rows);

	for (FGlobalAnimMontageDescriptorBase* MontageDescriptor : Rows)
	{
		MontageAssetPaths.Emplace(TPair<FName, uint64>(StructName, MontageDescriptor->GetKey()), MontageDescriptor->GetAssetPath());
	}
}

void UGlobalAnimMontageSubsystem::Finalize()
{
	if (bIsInitialized)
	{
		return;
	}

	// 게임 흐름을 관리하는 클래스에게 전역 애니메이션 몽타주 준비가 끝났음을 통지합니다.

	bIsInitialized = true;
}

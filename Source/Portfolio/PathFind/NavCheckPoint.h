#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavCheckPoint.generated.h"

USTRUCT(BlueprintType)
struct FConnectedNodeData
{
	GENERATED_USTRUCT_BODY()

public :
	// 연결된 Check Point의 Node Num
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConnectedNodeNum = INDEX_NONE;

	// 해당 노드로 연결된 비중 값
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Weight = 1;
};

USTRUCT(BlueprintType)
struct FCheckPointData
{
	GENERATED_USTRUCT_BODY()

public :
	// 체크 포인트의 고유 번호
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NodeNum = INDEX_NONE;

	// 연결된 체크 포인트 리스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FConnectedNodeData> ConnectedCheckPoint;

	// 체크 포인트 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Location = FVector::Zero();
};

// 플레이어 캐릭터가 길찾기를 시도할 때 경유할 지점
// 그래프의 노드 역할을 함
UCLASS(BlueprintType)
class PORTFOLIO_API ANavCheckPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	ANavCheckPoint();

protected:
	virtual void BeginPlay() override;
	
public:
	// 체크 포인트 데이터
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCheckPointData CheckPointData;
};

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Portfolio/PathFind/PathEdge.h"
#include "Portfolio/PathFind/NavCheckPoint.h"
#include "PathGraph.generated.h"

// Node와 Edge들의 정보로 경로 그래프를 구성합니다.

DEFINE_LOG_CATEGORY_STATIC(LogPathGraph, All, All);

UCLASS(BlueprintType)
class PORTFOLIO_API UPathGraph : public UObject
{
    GENERATED_BODY()

public:
	UPathGraph();
	~UPathGraph();

public :

	// 그래프를 생성
	UFUNCTION(BlueprintCallable) bool GenerateGraph();
	// 레벨에 배치된 체크포인트를 수집
	TArray<AActor*> GatherLevelCheckPoints();

	// Nav Point 데이터를 추가
	void AddNavPoint(FCheckPointData AddData);

	// 노드와 연결된 엣지 데이터 반환
	TSet<FPathEdge> GetLinkedEdges(int32 NodeNum) const;

	FORCEINLINE bool IsGraphEmpty() const { return EdgeData.IsEmpty(); }

	FORCEINLINE bool IsExist(int32 NodeNum) const { return EdgeData.Contains(NodeNum); }

	FORCEINLINE TArray<int32> GraphNodeList() const
	{
		TArray<int32> NodeList;
		EdgeData.GenerateKeyArray(NodeList);
		return NodeList;
	}

	FORCEINLINE FVector GetCheckPointLocation(int32 NodeNum)
	{
		if (CheckPointData.Contains(NodeNum))
		{
			return CheckPointData.Find(NodeNum)->Location;
		}
		return FVector::Zero();
	}

private:
	// 노드 데이터
	// 레벨에 배치된 노드 액터의 데이터를 수집
	TMap<int32 /*NodeNum*/, FCheckPointData>  CheckPointData;

	// Edge 데이터
	// 연결된 노드들의 정보
	// 연결 정보가 없는 노드는 제외
	TMap<int32 /*NodeNum*/, TSet<FPathEdge>> EdgeData;
};

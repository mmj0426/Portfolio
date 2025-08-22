#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Portfolio/PathFind/PathGraph.h"
#include "PathFinder.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogPathFinder, All, All);

UENUM(BlueprintType)
enum class EPathFindType : uint8
{
	E_Dijkstra					UMETA(DisplayName = "Dijkstra"),
	E_Distance					UMETA(DisplayName = "Distance"),
	E_DistanceAndDirection		UMETA(DisplayName = "Distance & Direction"),
};

UCLASS(BlueprintType)
class PORTFOLIO_API UPathFinder : public UObject
{
    GENERATED_BODY()

public :
	UPathFinder();
	~UPathFinder();

public :
	UFUNCTION(BlueprintCallable)
	void Init(float Radius);

	UFUNCTION(BlueprintCallable)
	TArray<int32> FindShortPath(EPathFindType FindType, class UPathGraph* PathGraph, const FVector& StartLocation, const FVector& EndLocation);

private :
	// 경로 그래프
	class UPathGraph* Graph;

	// 탐색할 반경
	float SearchRadius = 0.f;

	TArray<int32> CheckPath(const FVector& StartLocation, const FVector& EndLocation, const TArray<int32>& Path);

	int32 FindNearestNode(const FVector& Origin) const;

	// 구형 Trace 탐색으로 인접한 노드 탐색
	TArray<int32> FindAdjacentNode_SphereTrace(const FVector& SearchOrigin) const;

	// 그래프를 구성하는 모든 노드 거리 비교 탐색
	int32 FindNearestNode_AllGraphNode(const FVector& Origin) const;

	// 거리 기반 + 대상과의 방향을 고려한 노드 탐색
	bool FindNearestNode_WithDirection(int32& StartNode, int32& TargetNode, const FVector& StartOrigin, const FVector& TargetOrigin) const;

	// 최단 노드 경로 반환
	// Origin으로부터 TargetNode를 제외한 모든 노드가 연결되어있다고 가정 하에 계산
	TArray<int32> FindShortPath_Dijkstra(const FVector& Origin, const FVector& EndLoc) const;

	TArray<int32> Dijkstra(int32 StartPoint, int32 EndPoint, double& Cost) const;

	bool IsValidNavPath(int32 NodeNum) const;

#pragma region Debugging
	void DebugDrawPath(const TArray<int32>& Path) const;
#pragma endregion
};

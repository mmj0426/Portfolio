#pragma once

#include "CoreMinimal.h"
#include "PathEdge.generated.h"

// Graph의 노드를 Check Point라고 칭한다.
// CheckPoint_1과 CheckPoint_2 의 순서가 바뀌더라도 Edge 데이터는 같다.
// 따라서, Edge 생성 시 (CheckPoint_1 < CheckPoint_2) 상태로 대입되도록 약속한다.
// ex. FPath(1,2,Dist,Weight) == FPath(2,1,Dist,Weight)

USTRUCT(BlueprintType)
struct FPathEdge
{
	GENERATED_USTRUCT_BODY()

public:
	// 연결된 Check Point 중 작은 값을 가진 것
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckPoint_1 = INDEX_NONE;

	// 연결된 Check Point 중 큰 값을 가진 것
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckPoint_2 = INDEX_NONE;

	// 비용 값 (거리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double Cost = 0.f;

	// 비중 값 (Weight * 0.01 로 계산)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Weight = 1;

	FPathEdge() : CheckPoint_1(INDEX_NONE), CheckPoint_2(INDEX_NONE), Cost(0.f), Weight(1) {}

	FPathEdge(int32 InPoint_1, int32 InPoint_2, double InCost, int32 InWeight = 1)
	{
		if (InPoint_1 < InPoint_2)
		{
			CheckPoint_1 = InPoint_1;
			CheckPoint_2 = InPoint_2;
		}
		else
		{
			CheckPoint_1 = InPoint_2;
			CheckPoint_2 = InPoint_1;
		}

		Cost = InCost;
		Weight = InWeight;
	}

	bool operator==(const FPathEdge& Other) const
	{
		return CheckPoint_1 == Other.CheckPoint_1 && CheckPoint_2 == Other.CheckPoint_2;
	}
};

// TSet을 사용하기 위함
// Edge.CheckPoint_1과 Edge.CheckPoint_2 두 값을 각각 해시로 만든 뒤 HashCombine으로 하나로 합침.
// 두 체크포인트 쌍이 같으면 같은 해시를 반환하게 되어, TSet에서 중복 제거가 가능.
FORCEINLINE uint32 GetTypeHash(const FPathEdge& Edge)
{
	return HashCombine(GetTypeHash(Edge.CheckPoint_1), GetTypeHash(Edge.CheckPoint_2));
}

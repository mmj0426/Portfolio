#include "Portfolio/PathFind/PathGraph.h"
#include "Portfolio/PathFind/NavCheckPoint.h"

#include "Kismet/GameplayStatics.h"

UPathGraph::UPathGraph()
{
	CheckPointData.Empty();
	EdgeData.Empty();
}

UPathGraph::~UPathGraph()
{
	CheckPointData.Empty();
	EdgeData.Empty();
}

bool UPathGraph::GenerateGraph()
{
	CheckPointData.Empty();
	EdgeData.Empty();

	const TArray<AActor*>& LevelCheckPoints = GatherLevelCheckPoints();

	if (LevelCheckPoints.IsEmpty())
	{
		UE_LOG(LogPathGraph, Log, TEXT("No Check Point In Level"));
		return false;;
	}

	// 노드 데이터 수집
	for (AActor* Actor : LevelCheckPoints)
	{
		if (ANavCheckPoint* CheckPoint = Cast<ANavCheckPoint>(Actor))
		{
			CheckPointData.Emplace(CheckPoint->CheckPointData.NodeNum, CheckPoint->CheckPointData);
		}
	}

	// 그래프 구성
	for (const TPair<int32, FCheckPointData>& Data : CheckPointData)
	{
		AddNavPoint(Data.Value);
	}

	return true;
}

TArray<AActor*> UPathGraph::GatherLevelCheckPoints()
{
	TArray<AActor*> LevelCheckPoints;
	// 레벨에 배치된 CheckPoint 클래스 액터를 수집합니다.
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::GetAllActorsOfClass(World, ANavCheckPoint::StaticClass(), LevelCheckPoints);
	}
	else
	{
		UE_LOG(LogPathGraph, Error, TEXT("World is not valid"));
	}

	return LevelCheckPoints;
}

void UPathGraph::AddNavPoint(FCheckPointData AddData)
{
	for (const FConnectedNodeData& ConnectedData : AddData.ConnectedCheckPoint)
	{
		// 자기 자신과 연결되는 Edge는 무시
		if (AddData.NodeNum == ConnectedData.ConnectedNodeNum)
		{
			continue;
		}

		if (CheckPointData.Contains(ConnectedData.ConnectedNodeNum))
		{
			// 노드와 연결된 엣지 정보 추가
			FVector ConnectedNodeLoc = CheckPointData.Find(ConnectedData.ConnectedNodeNum)->Location;
			FPathEdge Edge(AddData.NodeNum, 
							ConnectedData.ConnectedNodeNum, 
							FVector::Dist(AddData.Location, ConnectedNodeLoc), 
							ConnectedData.Weight);

			EdgeData.FindOrAdd(Edge.CheckPoint_1).Emplace(Edge);
			EdgeData.FindOrAdd(Edge.CheckPoint_2).Emplace(Edge);
		}
	}
}

TSet<FPathEdge> UPathGraph::GetLinkedEdges(int32 NodeNum) const
{
	if (EdgeData.Contains(NodeNum))
	{
		return EdgeData.FindRef(NodeNum);
	}

	return TSet<FPathEdge>();
}

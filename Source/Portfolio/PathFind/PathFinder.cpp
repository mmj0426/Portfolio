#include "Portfolio/PathFind/PathFinder.h"
#include "Kismet/KismetSystemLibrary.h"

#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Portfolio/PortfolioCharacter.h"

#define MIN_PATH_THRESHOLD (50.f)

UPathFinder::UPathFinder()
{
	Graph = nullptr;
}

UPathFinder::~UPathFinder()
{
	Graph = nullptr;
}

void UPathFinder::Init(float Radius)
{
	SearchRadius = Radius;
}

TArray<int32> UPathFinder::FindShortPath(EPathFindType FindType, UPathGraph* PathGraph, const FVector& StartLocation, const FVector& EndLocation)
{
	Graph = PathGraph;
	if (!Graph || Graph->IsGraphEmpty())
	{
		UE_LOG(LogPathFinder, Error, TEXT("Graph Is not Valid"));
		return TArray<int32>();
	}

	switch (FindType)
	{
		case EPathFindType::E_Dijkstra: return CheckPath(StartLocation, EndLocation, FindShortPath_Dijkstra(StartLocation, EndLocation));
		case EPathFindType::E_Distance:
		{
			double Cost = 0.f;
			return CheckPath(StartLocation, EndLocation, Dijkstra(FindNearestNode(StartLocation), FindNearestNode(EndLocation), Cost));
		}
		case EPathFindType::E_DistanceAndDirection:
		{
			int32 StartNode = INDEX_NONE;
			int32 TargetNode = INDEX_NONE;
			double Cost = 0.f;

			if (FindNearestNode_WithDirection(StartNode, TargetNode, StartLocation, EndLocation))
			{
				return CheckPath(StartLocation, EndLocation, Dijkstra(StartNode, TargetNode, Cost));
			}
		}
		default:UE_LOG(LogPathFinder, Warning, TEXT("Unknown path find type")); break;
	}

	return TArray<int32>();
}

TArray<int32> UPathFinder::CheckPath(const FVector& StartLocation, const FVector& EndLocation, const TArray<int32>& Path)
{	
	if (Path.IsEmpty())
	{
		return TArray<int32>();
	}

	TArray<int32> Result = Path;

	if (Result.Num() >= 2)
	{
		FVector2D FirstLoc = FVector2D(Graph->GetCheckPointLocation(Path[0]));
		FVector2D SecondLoc = FVector2D((Path.IsValidIndex(1)) ? Graph->GetCheckPointLocation(Path[1]) : EndLocation);

		FVector2D DirToFirst = (FirstLoc - FVector2D(StartLocation)).GetSafeNormal();
		FVector2D DirToSecond = (SecondLoc - FVector2D(StartLocation)).GetSafeNormal();

		// (시작 지점 부터 첫번째 노드까지의 방향) 과 (시작 지점 부터 두번째 노드까지의 방향) 이 이루는 각이 둔각 (Dot ≤ 0) 이라면 첫번째 노드는 거치지 않는다.
		float Dot = FVector2D::DotProduct(DirToFirst, DirToSecond);
		if (Dot <= 0.f)
		{
			Result.RemoveAt(0);
		}
	}

	if (Result.IsEmpty())
	{
		return Result;
	}

	// 마지막 노드로 향하기 전, (목적지까지의 거리) ≤ (마지막 노드까지의 거리) 일 경우 마지막 노드는 거치지 않는다.
	int32 LastNodeIndex = Result.Num() - 1;
	FVector2D LastNodeLoc = FVector2D(Graph->GetCheckPointLocation(Result[LastNodeIndex]));
	FVector2D ReferenceLoc = (Result.Num() > 1) ? FVector2D(Graph->GetCheckPointLocation(Result[LastNodeIndex - 1])) : FVector2D(StartLocation);

	double DistanceToLastNode = FVector2D::Distance(LastNodeLoc, ReferenceLoc);
	double DistanceToDestination = FVector2D::Distance(FVector2D(EndLocation), ReferenceLoc);

	if (DistanceToDestination <= DistanceToLastNode)
	{
		Result.RemoveAt(LastNodeIndex);
	}

	return Result;
}

int32 UPathFinder::FindNearestNode(const FVector& Origin) const
{
	int32 ResultNode = INDEX_NONE;

	// 인접 노드 검색 (범위 반경 탐색)
	TArray<int32> NearEndNodes = FindAdjacentNode_SphereTrace(Origin);
	double Distance = TNumericLimits<double>::Max();

	if (NearEndNodes.IsEmpty())
	{
		// 범위 탐색 실패 시 모든 노드 탐색
		return FindNearestNode_AllGraphNode(Origin);
	}
	else
	{
		for(int32 NodeNum : NearEndNodes)
		{
			FVector NodeLocation = Graph->GetCheckPointLocation(NodeNum);
			double NewDistance = FVector2D::Distance(FVector2D(NodeLocation), FVector2D(Origin));

			if (NewDistance < Distance)
			{
				Distance = NewDistance;
				ResultNode = NodeNum;
			}
		}
	}

	return ResultNode;
}

TArray<int32> UPathFinder::FindAdjacentNode_SphereTrace(const FVector& SearchOrigin) const
{
	// CheckPoint ObjectType
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {EObjectTypeQuery::ObjectTypeQuery8};

	TArray<FHitResult> HitResults;
	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		this,
		SearchOrigin,
		SearchOrigin,
		SearchRadius,
		ObjectTypes,
		false, TArray<AActor*>(), EDrawDebugTrace::None, HitResults, true, FLinearColor::Red, FLinearColor::Green, 10.f);

	if (false == bHit)
	{
		return TArray<int32>();	
	}

	TArray<int32> Results;
	for (const FHitResult& Hit : HitResults)
	{
		if (ANavCheckPoint* CheckPoint = Cast<ANavCheckPoint>(Hit.GetActor()))
		{
			Results.Emplace(CheckPoint->CheckPointData.NodeNum);
		}
	}

	return Results;
}

int32 UPathFinder::FindNearestNode_AllGraphNode(const FVector& Origin) const
{
	double Cost = TNumericLimits<double>::Max();
	int32 NearestNode = INDEX_NONE;

	TArray<int32> AllNodeList = Graph->GraphNodeList();
	for (int32 NodeNum : AllNodeList)
	{
		// 목표 노드는 목표 지점으로부터 가장 가까운 노드
		FVector NodeLoc = Graph->GetCheckPointLocation(NodeNum);
		double DistCost = FVector2D::Distance(FVector2D(NodeLoc), FVector2D(Origin));

		if (DistCost < Cost)
		{
			Cost = DistCost;
			NearestNode = NodeNum;
		}
	}

	return NearestNode;
}

bool UPathFinder::FindNearestNode_WithDirection(int32& StartNode, int32& TargetNode, const FVector& StartOrigin, const FVector& TargetOrigin) const
{
	StartNode = INDEX_NONE;
	TargetNode = INDEX_NONE;

	double StartCost = TNumericLimits<double>::Max();
	double TargetCost = TNumericLimits<double>::Max();

	FVector2D Start2D = FVector2D(StartOrigin);
	FVector2D Target2D = FVector2D(TargetOrigin);

	FVector2D StartToTarget = (Target2D - Start2D).GetSafeNormal();

	TArray<int32> AllNodeList = Graph->GraphNodeList();
	for (int32 NodeNum : AllNodeList)
	{
		FVector NodeLoc = Graph->GetCheckPointLocation(NodeNum);
		FVector2D NodeLoc2D = FVector2D(NodeLoc.X, NodeLoc.Y);

		FVector2D StartToNode = (NodeLoc2D - Start2D).GetSafeNormal();
		FVector2D TargetToNode = (NodeLoc2D - Target2D).GetSafeNormal();

		// 충분히 먼 거리에 위치해 있을 경우 거리의 영향을 더 받게 됨.
		// 거리가 짧은 경우에는 방향의 영향을 더 받게 됨.
		double NewStartCost = FVector2D::Distance(NodeLoc2D, Start2D) * (1 - FVector2D::DotProduct(StartToNode, StartToTarget));
		double NewTargetCost = FVector2D::Distance(NodeLoc2D, Target2D) * (1 - FVector2D::DotProduct(TargetToNode, -1 * StartToTarget));

		if (NewStartCost < StartCost)
		{
			StartCost = NewStartCost;
			StartNode = NodeNum;
		}

		if (NewTargetCost < TargetCost)
		{
			TargetCost = NewTargetCost;
			TargetNode = NodeNum;
		}
	}
	return (StartNode != INDEX_NONE) && (TargetNode != INDEX_NONE);
	//return (StartNode < 0) || (TargetNode < 0);
}

TArray<int32> UPathFinder::FindShortPath_Dijkstra(const FVector& Origin, const FVector& EndLoc) const
{
	int32 TargetNodeNum = FindNearestNode(EndLoc);

	// 범위 탐색 후 없을 경우 모든 노드
	TArray<int32> NearStartNodes = FindAdjacentNode_SphereTrace(Origin);
	if (NearStartNodes.IsEmpty())
	{
		NearStartNodes = Graph->GraphNodeList();
	}

	double PathCost = TNumericLimits<double>::Max();
	TArray<int32> Result;

	for (int32 NodeNum : NearStartNodes)
	{
		double Cost;
		TArray<int32> Path = Dijkstra(NodeNum, TargetNodeNum, Cost);

		if (Path.IsEmpty())
		{
			continue;
		}

		// 현재 위치 ~ 첫 경유지 까지의 경로 존재 여부
		if (false == IsValidNavPath(Path[0]))
		{
			continue;
		}

		// 더 짧은 경우 갱신
		if ((Cost >= 0) && (Cost < PathCost))
		{
			PathCost = Cost;
			Result = Path;
		}
	}

	return Result;
}

bool UPathFinder::IsValidNavPath(int32 NodeNum) const
{
	if (false == Graph->IsExist(NodeNum))
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World))
		{
			if (APortfolioCharacter* PlayerCharacter = Cast<APortfolioCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)))
			{
				FPathFindingQuery Query;

				Query.NavData = NavSystem->GetDefaultNavDataInstance();
				Query.StartLocation = PlayerCharacter->GetCharacterLocation();
				Query.EndLocation = Graph->GetCheckPointLocation(NodeNum);
				Query.QueryFilter = Query.NavData.IsValid() ? Query.NavData->GetDefaultQueryFilter() : nullptr;

				FPathFindingResult Result = NavSystem->FindPathSync(Query);
				return (false == Result.IsPartial()) && (Result.Result == ENavigationQueryResult::Success);
			}
		}
	}

	return false;
}

TArray<int32> UPathFinder::Dijkstra(int32 StartPoint, int32 EndPoint, double& Cost) const
{
	Cost = 0.f;

	// 그래프에 출발/도착 노드 존재 판단
	if (false == Graph->IsExist(StartPoint) || false == Graph->IsExist(EndPoint))
	{
		return TArray<int32>();
	}

	// 출발지와 목적지가 같으면 바로 반환
	if (StartPoint == EndPoint)
	{
		return {StartPoint};
	}

	double DirectDistance = FVector::Dist(Graph->GetCheckPointLocation(StartPoint), Graph->GetCheckPointLocation(EndPoint));

	// 출발지와 목적지의 거리가 충분히 가깝다면 바로 반환
	if (DirectDistance < MIN_PATH_THRESHOLD)
	{
		return {StartPoint};
	}

	TMap<int32, double> Distance;

	// 방문하지 않은 노드는 -1로 초기화
	{
		TArray<int32> AllNodeList = Graph->GraphNodeList();
		for (int32 NodeNum : AllNodeList)
		{
			Distance.Emplace(NodeNum, TNumericLimits<double>::Max());
		}
		// 시작 지점은 0
		Distance[StartPoint] = 0.f;
	}

	// 경로 추적
	TMap<int32, int32> PreviousNodes;

	// 우선순위 큐
	TArray<TPair<double, int32>> MinHeap;
	MinHeap.HeapPush(TPair<double, int32>(0.f, StartPoint), TLess<TPair<double, int32>>());

	while (false == MinHeap.IsEmpty())
	{
		// 현재 최단 거리 노드
		TPair<double, int32> CurrentPoint;
		MinHeap.HeapPop(CurrentPoint, TLess<TPair<double, int32>>());

		int32 CurrentNode = CurrentPoint.Value;

		// 목표 노드에 도달 시 중지
		if (CurrentNode == EndPoint)
		{
			break;
		}

		// 인접 노드 탐색
		const TSet<FPathEdge>& LinkedEdgeList= Graph->GetLinkedEdges(CurrentNode);
		for (const FPathEdge& Edge : LinkedEdgeList)
		{
			int32 NeighborNode = (Edge.CheckPoint_1 == CurrentNode) ? Edge.CheckPoint_2 : Edge.CheckPoint_1;

			// 인접노드에서 StartPoint로 가는 비용은 계산은 불필요
			if (false == Distance.Contains(CurrentNode) || NeighborNode == StartPoint)
			{
				continue;
			}

			double NewDistance = Distance[CurrentNode] + (Edge.Cost * Edge.Weight);
			if (NewDistance < Distance[NeighborNode])
			{
				Distance[NeighborNode] = NewDistance;
				PreviousNodes.Emplace(NeighborNode, CurrentNode);

				MinHeap.HeapPush(TPair<double, int32>(NewDistance, NeighborNode), TLess<TPair<double, int32>>());
			}
		}
	}

	// 최단 경로 추적
	TArray<int32> Path;
	{
		int32 PathNode = EndPoint;
		while (PreviousNodes.Contains(PathNode))
		{
			Path.Emplace(PathNode);
			PathNode = PreviousNodes[PathNode];

			// 경로가 정상적으로 도달하면 중단
			if (PathNode == StartPoint) 
			{
				Path.Emplace(StartPoint);
				break;
			}
		}

		Algo::Reverse(Path);
		Cost = Distance.Contains(EndPoint) ? Distance[EndPoint] : INDEX_NONE;
	}

	return Path;
}

#pragma region Debugging

void UPathFinder::DebugDrawPath(const TArray<int32>& Path) const
{
#if WITH_EDITOR
	for (int32 i = 0; i < Path.Num() - 1; ++i)
	{
		FVector Start = Graph->GetCheckPointLocation(Path[i]);
		FVector End = Graph->GetCheckPointLocation(Path[i + 1]);
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5.0f, 0, 2.0f);
	}
#endif
}

#pragma endregion

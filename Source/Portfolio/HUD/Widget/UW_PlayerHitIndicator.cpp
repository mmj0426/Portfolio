#include "HUD/Widget/UW_PlayerHitIndicator.h"

#include "GameFramework/Character.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

#include "Kismet/GameplayStatics.h"

#pragma region UUW_PlayerHitIndicator
void UUW_PlayerHitIndicator::UpdateAngle(FVector TargetLocation)
{
	UWorld* World = GetWorld();
	if (false == IsValid(ParentPanel) || false == IsValid(World))
	{
		return;
	}

	if (APlayerController* Controller = UGameplayStatics::GetPlayerController(World, 0))
	{
		FVector2D UpVector(0.f, -1.f);

		// 대상의 좌표를 스크린 좌표로 변환
		FVector2D TargetScreenLoc;
		Controller->ProjectWorldLocationToScreen(TargetLocation, TargetScreenLoc);

		// 플레이어의 좌표를 스크린 좌표로 변환
		FVector2D PlayerScreenLoc;
		Controller->ProjectWorldLocationToScreen(Controller->GetCharacter()->GetActorLocation(), PlayerScreenLoc);

		// 플레이어의 좌표가 대상을 바라보고있는 벡터
		FVector2D LookAtTarget = TargetScreenLoc - PlayerScreenLoc;
		LookAtTarget.Normalize();

		// 내적을 통한 벡터 사이의 각도 구하기 / 외적을 통한 방향 판단
		float Cross = FVector2D::CrossProduct(UpVector, LookAtTarget);
		float Dot = FVector2D::DotProduct(UpVector, LookAtTarget);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

		float TargetAngle = Cross < 0 ? 360.f - Angle : Angle;
		ParentPanel->SetRenderTransformAngle(TargetAngle);

		StartAnimation();
	}
}
#pragma endregion

//============================================================================================================

#pragma region UUW_PlayerHitIndicatorPool

bool UUW_PlayerHitIndicatorPool::Initialize()
{
	return Super::Initialize();
}

void UUW_PlayerHitIndicatorPool::NativeConstruct()
{
	Super::NativeConstruct();

	// Check Widget Binding
	if (false == IsValid(ParentPanel))
	{
		UE_LOG(LogTemp, Error, TEXT("UUW_PlayerHitIndicator ParentPanel is not Valid"))
		RemoveFromParent();
	}
	// Initialize Pool
	else
	{
		for (int32 Index = 0; Index < PoolInitValue; Index++)
		{
			IndicatorWidgetPool.Emplace(CreateIndicator());
		}
	}
}

void UUW_PlayerHitIndicatorPool::NativeDestruct()
{
	Super::NativeDestruct();
}

UUW_PlayerHitIndicator* UUW_PlayerHitIndicatorPool::CreateIndicator()
{
	UUW_PlayerHitIndicator* CreatedWidget = CreateWidget<UUW_PlayerHitIndicator>(this, IndicatorWidgetClass);
	CreatedWidget->OnFinishedAnimation.AddDynamic(this, &UUW_PlayerHitIndicatorPool::ReturnIndicator);
	CreatedWidget->SetVisibility(ESlateVisibility::Collapsed);
	ParentPanel->AddChild(CreatedWidget);

	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(CreatedWidget->Slot))
	{
		CanvasPanelSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasPanelSlot->SetOffsets(FMargin());
	}

	return CreatedWidget;
}

UUW_PlayerHitIndicator* UUW_PlayerHitIndicatorPool::GetIndicatorWidget()
{
	UUW_PlayerHitIndicator* ReturnWidget = nullptr;

	if (IndicatorWidgetPool.IsEmpty())
	{
		ReturnWidget = CreateIndicator();
	}
	else
	{
		int32 LastIndex = IndicatorWidgetPool.Num()-1;
		ReturnWidget = IndicatorWidgetPool[LastIndex];
		IndicatorWidgetPool.RemoveAt(LastIndex);
	}

	// Active Inidicator
	ReturnWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ActiveIndicator.Emplace(ReturnWidget);

	return ReturnWidget;
}

void UUW_PlayerHitIndicatorPool::ReturnIndicator(UUW_PlayerHitIndicator* IndicatorWidget)
{
	if (false == IsValid(IndicatorWidget))
	{
		return;
	}

	if (IndicatorWidgetPool.Num() < PoolMaxValue)
	{
		IndicatorWidget->SetVisibility(ESlateVisibility::Collapsed);
		ActiveIndicator.Remove(IndicatorWidget);
		IndicatorWidgetPool.Emplace(IndicatorWidget);
	}
	else
	{
		ActiveIndicator.Remove(IndicatorWidget);
		IndicatorWidget->RemoveFromParent();
	}
}

void UUW_PlayerHitIndicatorPool::ResetIndicator()
{
	for (UUW_PlayerHitIndicator* Widget : ActiveIndicator)
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
		IndicatorWidgetPool.Emplace(Widget);
	}
	ActiveIndicator.Empty();
}

void UUW_PlayerHitIndicatorPool::ShowIndicator(FVector TargetLocation)
{
	if (UUW_PlayerHitIndicator* IndicatorWidget = GetIndicatorWidget())
	{
		IndicatorWidget->UpdateAngle(TargetLocation);
	}
}

#pragma endregion
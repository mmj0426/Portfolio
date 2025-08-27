#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PlayerHitIndicator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinishedAnimation, class UUW_PlayerHitIndicator*, IndicatorWidget);

UCLASS(BlueprintType)
class PORTFOLIO_API UUW_PlayerHitIndicator : public UUserWidget
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable) void UpdateAngle(FVector TargetLocation);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent) void StartAnimation();
	UFUNCTION(BlueprintCallable) FORCEINLINE void EndAnimation() { OnFinishedAnimation.Broadcast(this); }
	

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnFinishedAnimation OnFinishedAnimation;

#pragma region Bind Widget
public :
	// 최상위 캔버스 패널 위젯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UCanvasPanel* ParentPanel;
#pragma endregion
};


UCLASS(BlueprintType)
class PORTFOLIO_API UUW_PlayerHitIndicatorPool : public UUserWidget
{
	GENERATED_BODY()
	
public :
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected :
	// 생성될 표시 위젯 클래스
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<class UUW_PlayerHitIndicator> IndicatorWidgetClass;

	// 풀 초기 생성 개수
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	int32 PoolInitValue = 1;

	// 풀에 들어갈 수 있는 최대 개수
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	int32 PoolMaxValue = 10;

	// 생성된 위젯 리스트
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TArray<class UUW_PlayerHitIndicator*> IndicatorWidgetPool;

	// 활성화 중인 위젯들
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	TArray<class UUW_PlayerHitIndicator*> ActiveIndicator;


	class UUW_PlayerHitIndicator* CreateIndicator();

	// 풀에서 반환 및 생성
	UFUNCTION(BlueprintCallable) class UUW_PlayerHitIndicator* GetIndicatorWidget();
	// 풀에 반환
	UFUNCTION(BlueprintCallable) void ReturnIndicator(UUW_PlayerHitIndicator* IndicatorWidget);
	// 모든 위젯 반환
	UFUNCTION(BlueprintCallable) void ResetIndicator();

public :
	UFUNCTION(BlueprintCallable) void ShowIndicator(FVector TargetLocation);

#pragma region Bind Widget
public :
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UCanvasPanel* ParentPanel;
#pragma endregion
};

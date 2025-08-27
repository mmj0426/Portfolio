#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "InputInteractor.generated.h"

// 축 처리 모드
UENUM(BlueprintType)
enum class EAxisHandlingMode : uint8
{
    None UMETA(DisplayName = "None"),
    Translate UMETA(DisplayName = "Translate"),
    Rotate UMETA(DisplayName = "Rotate")
};

USTRUCT(BlueprintType)
struct FInputInteractionData
{
	GENERATED_USTRUCT_BODY()

public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAxisHandlingMode AxisHandlingModeX = EAxisHandlingMode::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAxisHandlingMode AxisHandlingModeY = EAxisHandlingMode::None;

	// 델타 값에 곱할 배수(회전), 기본값은 단위 벡터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
	FVector2D RotationDeltaMultiplier = FVector2D::UnitVector;	

	// 회전 제약 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraints")
	bool bUseRotationConstraints = false; 

	// 회전에 대한 최소 제약
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraints|Rotation")
	FVector2D RotationMinConstraints = FVector2D::ZeroVector; 

	// 회전에 대한 최대 제약
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraints|Rotation")
	FVector2D RotationMaxConstraints = FVector2D::ZeroVector; 

	// 델타 값에 곱할 배수(이동), 기본값은 단위 벡터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Multiplier")
	FVector2D TranslationDeltaMultipliers = FVector2D::UnitVector;	

	// 이동 제약 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Constraints")
	bool bUseTranslationConstraints = false; 

	// 최소 이동 패딩
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Translation")
	FVector2D TranslationMinPadding = FVector2D::ZeroVector; 

	// 최대 이동 패딩
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Translation")
	FVector2D TranslationMaxPadding = FVector2D::ZeroVector; 

	// 줌인아웃 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	bool bEnableZoom = false; 

	// 마우스 위치 기반 줌 사용여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	bool bEnableMouseBasedZoom = false; 

	// 기본 줌 스텝
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	int DefaultZoomStep = 0; 

	// 최소 줌 스텝
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	int MinZoomStep = 0; 

	// 최대 줌 스텝
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	int MaxZoomStep = 1; 

	// 스텝당 이동량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	int ZoomStepIncrement = 100; 

	// 보간 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zoom")
	float ZoomInterpolationSpeed = 1.f; 

	// 씬 뷰 높이. 설정값보다 클수록 줌 아웃 시 카메라 여백 처리가 가능하다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")	
	float SceneViewHeight = 0.0f;

	// 줌 기반 이동 제약 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	bool bUseZoomConstraint = true; 
};

UCLASS(BlueprintType)
class PORTFOLIO_API UInputInteractor : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public :
	UInputInteractor();
	~UInputInteractor();

    UFUNCTION(BlueprintNativeEvent/*BlueprintImplementableEvent*/)
    void Tick(float DeltaTime) override;
    virtual void Tick_Implementation(float DeltaTime);


protected :
	bool bIsActor = false;
	bool bIsSetupComplete = false;			// 초기화 여부 플래그
	/*모든 타깃의 초기 Transform 캐시
	  Transform.Rotation = RotationTarget의 Rotation
	  Transform.Location.X, Y = TranslateTarget의 Location Y, Z (상하좌우)
	  Transform,Location.Z = ZoomControlTarget의 Location.Z */ 
	FTransform InitialTargetsTransform;		

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn = true), Category = Interaction)
	FInputInteractionData InteractionData;

	float MaxZoomDistance = 0.0f;		// 최대 줌 이동 거리
	int CurrentZoomStep = 0;			// 현재 줌 스텝
	float FinalZoomAmount = 0.f;		// 현재 줌 스텝에 따른 최종 이동량
	float AccumulatedZoomAmount = 0.f;	// 현재까지 계산하여 적용된 줌 스텝 이동량
	float ZoomTargetActorSize = 90.f;	// 줌 대상 사이즈

	FVector2D FinalTranslationMinConstraints = FVector2D::ZeroVector; // 최종 최소 제약
	FVector2D FinalTranslationMaxConstraints = FVector2D::ZeroVector; // 최종 최대 제약

	FVector2D AccumulatedInputDelta = FVector2D::ZeroVector;
	FVector2D ProcessedInputDelta = FVector2D::ZeroVector;

public :
	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool IsSetupComplete(bool DisplayLog = false) const;

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void Setup();

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void ResetRotationTarget();

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void ResetTranslationTarget();

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void ResetZoomControlTarget();

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void ResetAllTargets();

protected:
	void ApplyRotationConstraints(FVector2D& RotationDelta);
	void ApplyTranslationConstraints(FVector2D& TranslationDelta);
	void UpdateZoomInterpolation(float DeltaTime);
	void UpdateZoomStep(int Delta);
	void AdjustVerticalBounds();
	float CalculateVisibleHeight();


	#pragma region FTickableGameObject
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickableGameObject", meta = (AllowPrivateAccess = "true"))
    bool _IsTickable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickableGameObject", meta = (AllowPrivateAccess = "true"))
    bool _IsTickableInEditor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TickableGameObject", meta = (AllowPrivateAccess = "true"))
    bool _IsTickableWhenPaused = false;

public:

	bool IsTickable() const override final;

    bool IsTickableInEditor() const override final { return _IsTickableInEditor; }
    bool IsTickableWhenPaused() const override final { return _IsTickableWhenPaused; }
    TStatId GetStatId() const override final { return TStatId(); }

#pragma endregion


public :
	UFUNCTION(BlueprintCallable) void OnInputMoved(FVector2D DeltaPosition);
	UFUNCTION(BlueprintCallable) void OnInputWheelEvent(float Delta);
	UFUNCTION(BlueprintCallable)
	void InitializeInteractionData(FInputInteractionData Data) { InteractionData = Data; }
	UFUNCTION(BlueprintCallable) void StartInputInteraction() { _IsTickable = true; }
	UFUNCTION(BlueprintCallable) void StopInputInteraction() { _IsTickable = false; }
};

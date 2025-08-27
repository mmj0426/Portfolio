#include "SceneActor/InputInteractor.h"
#include "SceneActor/InterfaceInputInteractor.h"
#include "Kismet/GameplayStatics.h"

UInputInteractor::UInputInteractor()
{
	bIsActor = Cast<AActor>(GetOuter()) ? true : false;
}

UInputInteractor::~UInputInteractor()
{
	_IsTickable = false;
}

void UInputInteractor::Tick_Implementation(float DeltaTime)
{
	if (bIsSetupComplete)
	{
		// 보간 처리
		UpdateZoomInterpolation(DeltaTime);
	}
}

void UInputInteractor::Setup()
{
	// 보간 처리 관련 변수 초기화
	CurrentZoomStep = InteractionData.DefaultZoomStep;
	FinalZoomAmount = 0.f;
	AccumulatedZoomAmount = 0.f; 
	InitialTargetsTransform = FTransform();
	MaxZoomDistance = (InteractionData.ZoomStepIncrement * InteractionData.MaxZoomStep);

	if (InteractionData.AxisHandlingModeX == EAxisHandlingMode::Rotate || InteractionData.AxisHandlingModeY == EAxisHandlingMode::Rotate)
	{
		if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetRotationTarget(GetOuter()))
		{
			InitialTargetsTransform.SetRotation(Target->GetComponentRotation().Quaternion());
		}
	}

	if (InteractionData.AxisHandlingModeX == EAxisHandlingMode::Translate || InteractionData.AxisHandlingModeY == EAxisHandlingMode::Translate)
	{
		if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetTranslationTarget(GetOuter()))
		{
			FVector Location = InitialTargetsTransform.GetLocation();
			Location.X = Target->GetComponentLocation().Y;
			Location.Y = Target->GetComponentLocation().Z;
			InitialTargetsTransform.SetLocation(Location);
		}
	}

	if (InteractionData.bEnableZoom)
	{
		if (USceneComponent* CameraTarget = IInterfaceInputInteractor::Execute_GetZoomControlTarget(GetOuter()))
		{
			FVector Location = InitialTargetsTransform.GetLocation();
			Location.Z = CameraTarget->GetComponentLocation().X;
			InitialTargetsTransform.SetLocation(Location);
		}

		if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetZoomControlTarget(GetOuter()))
		{
			// 보간 없이 바로 적용
			AccumulatedZoomAmount = FinalZoomAmount = (CurrentZoomStep * InteractionData.ZoomStepIncrement) * Target->GetForwardVector().X;
		}

		// 줌 사용을 설정했는데 SceneViewHeight 값이 0 이라면 초기 거리값을 기준으로 높이를 자동으로 설정.
		if (InteractionData.SceneViewHeight == 0)
		{
			InteractionData.SceneViewHeight = CalculateVisibleHeight();
		}
	}

	bIsSetupComplete = true;
	AdjustVerticalBounds();
	ResetZoomControlTarget();
}

bool UInputInteractor::IsSetupComplete(bool DisplayLog) const
{
	if (DisplayLog && false == bIsSetupComplete)
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseSceneCapture is not initialized. Call CacheInitialTransforms first."));
	}

	return bIsSetupComplete;
}

void UInputInteractor::ResetRotationTarget()
{
	if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetRotationTarget(GetOuter()))
	{
		if (IsSetupComplete(true))
		{
			Target->SetWorldRotation(InitialTargetsTransform.GetRotation());
		}
	}
}

void UInputInteractor::ResetTranslationTarget()
{
	//if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetTranslationTarget(GetOuter()))
	//{
	//	if (IsSetupComplete(true))
	//	{
	//		FVector InitLocation = InitialTargetsTransform.GetLocation();
	//		InitLocation.Z = Target->GetComponentLocation().Z;
	//		Target->SetWorldLocation(InitLocation);
	//	}
	//}

	if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetTranslationTarget(GetOuter()))
	{
		if (IsSetupComplete(true))
		{
			// InitialTargetsTransform에서 올바른 초기 위치 복원
			FVector ResetLocation = Target->GetComponentLocation();
			ResetLocation.Y = InitialTargetsTransform.GetLocation().X;
			ResetLocation.Z = InitialTargetsTransform.GetLocation().Y;
			Target->SetWorldLocation(ResetLocation);
		}
	}
}

void UInputInteractor::ResetZoomControlTarget()
{
	if (false == InteractionData.bEnableZoom)
	{
		return;
	}
	
	if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetZoomControlTarget(GetOuter()))
	{
		if (IsSetupComplete(true))
		{
			FVector InitLocation = Target->GetComponentLocation();
			InitLocation.X = InitialTargetsTransform.GetLocation().Z + (CurrentZoomStep * InteractionData.ZoomStepIncrement) * Target->GetForwardVector().X;
			Target->SetWorldLocation(InitLocation);

			// 줌 컨트롤 타깃이 초기화 되면 이동 대상도 상하 제약이 있는 경우 위치를 보정해줘야 한다.
			AdjustVerticalBounds();

			if (USceneComponent* TranslationTarget = IInterfaceInputInteractor::Execute_GetTranslationTarget(GetOuter()))
			{
				FVector2D CalcTranslate = FVector2D(TranslationTarget->GetComponentLocation().Y, TranslationTarget->GetComponentLocation().Z);
				ApplyTranslationConstraints(CalcTranslate);
				TranslationTarget->SetWorldLocation(FVector(TranslationTarget->GetComponentLocation().X, CalcTranslate.X, CalcTranslate.Y));
			}
		}
	}
}

void UInputInteractor::ResetAllTargets()
{
	ResetRotationTarget();
	ResetTranslationTarget();
	ResetZoomControlTarget();
}

void UInputInteractor::ApplyRotationConstraints(FVector2D& RotationDelta)
{
	if (InteractionData.bUseRotationConstraints)
	{
		if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetRotationTarget(GetOuter()))
		{
			FVector2D CurrentRotation = FVector2D(Target->GetComponentRotation().Yaw, Target->GetComponentRotation().Pitch);
			FVector2D MinDelta = InteractionData.RotationMinConstraints - CurrentRotation;
			FVector2D MaxDelta = InteractionData.RotationMaxConstraints - CurrentRotation;

			if (InteractionData.AxisHandlingModeX == EAxisHandlingMode::Rotate)
			{
				RotationDelta.X = FMath::Clamp(RotationDelta.X, MinDelta.X, MaxDelta.X);
			}

			if (InteractionData.AxisHandlingModeY == EAxisHandlingMode::Rotate)
			{
				RotationDelta.Y = FMath::Clamp(RotationDelta.Y, MinDelta.Y, MaxDelta.Y);
			}
		}
	}
}

void UInputInteractor::ApplyTranslationConstraints(FVector2D& TranslationDelta)
{
	if (InteractionData.bUseTranslationConstraints)
	{
		if (InteractionData.AxisHandlingModeX == EAxisHandlingMode::Translate)
		{
			TranslationDelta.X = FMath::Clamp(TranslationDelta.X, FinalTranslationMinConstraints.X, FinalTranslationMaxConstraints.X);
		}

		if (InteractionData.AxisHandlingModeY == EAxisHandlingMode::Translate)
		{
			TranslationDelta.Y = FMath::Clamp(TranslationDelta.Y, FinalTranslationMinConstraints.Y, FinalTranslationMaxConstraints.Y);
		}
	}
}

void UInputInteractor::UpdateZoomInterpolation(float DeltaTime)
{
	USceneComponent* ZoomTarget = IInterfaceInputInteractor::Execute_GetZoomControlTarget(GetOuter());

	// 줌 기능이 활성화 되어있고 목표와 현재가 다를때만 동작.
	if (false == InteractionData.bEnableZoom || FinalZoomAmount == AccumulatedZoomAmount || ZoomTarget == nullptr)
	{
		return;
	}

	// 카메라의 새로운 위치 계산
	float CalcProcessedInputDelta = FMath::FInterpTo(AccumulatedZoomAmount, FinalZoomAmount, DeltaTime, InteractionData.ZoomInterpolationSpeed);
	FVector ZoomTargetLocation = ZoomTarget->GetComponentLocation();
	ZoomTargetLocation.X = InitialTargetsTransform.GetLocation().Z + CalcProcessedInputDelta;

	// 카메라 위치 업데이트
	ZoomTarget->SetWorldLocation(ZoomTargetLocation);

	AccumulatedZoomAmount = CalcProcessedInputDelta;

	// 상하 이동 범위 재계산
	AdjustVerticalBounds();

	if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetTranslationTarget(GetOuter()))
	{
		FVector2D CalcTranslate = FVector2D(Target->GetComponentLocation().Y, Target->GetComponentLocation().Z);
		ApplyTranslationConstraints(CalcTranslate);
		Target->SetWorldLocation(FVector(Target->GetComponentLocation().X, CalcTranslate.X, CalcTranslate.Y));
	}

	if (InteractionData.bEnableMouseBasedZoom)
	{
		// 현재 처리된 입력과 누적된 입력 사이를 보간하여 새로운 입력 델타를 계산합니다.
		FVector2D CalcedProcessedInputDelta = FMath::Vector2DInterpTo(ProcessedInputDelta, AccumulatedInputDelta, DeltaTime, InteractionData.ZoomInterpolationSpeed);

		// 계산된 델타를 이용하여 입력 이동 처리를 합니다.
		OnInputMoved(ProcessedInputDelta - CalcedProcessedInputDelta);

		// 처리된 입력 델타를 업데이트합니다.
		ProcessedInputDelta = CalcedProcessedInputDelta;

		// 계산된 델타와 누적된 델타가 거의 동일한 경우, 누적된 입력과 처리된 입력을 초기화합니다.
		if (AccumulatedInputDelta.Equals(ProcessedInputDelta, 0.001f))
		{
			AccumulatedInputDelta = FVector2D::ZeroVector;
			ProcessedInputDelta = FVector2D::ZeroVector;
		}
	}
}

void UInputInteractor::UpdateZoomStep(int Delta)
{
	if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetZoomControlTarget(GetOuter()))
	{
		int ChangeZoomStep = FMath::Clamp(CurrentZoomStep + Delta, InteractionData.MinZoomStep, InteractionData.MaxZoomStep);
		if (CurrentZoomStep == ChangeZoomStep)
		{
			return;
		}

		CurrentZoomStep = ChangeZoomStep;
		FinalZoomAmount = (CurrentZoomStep * InteractionData.ZoomStepIncrement) * Target->GetForwardVector().X;

		if (Delta <= 0.f || false == InteractionData.bEnableMouseBasedZoom)
		{
			return;
		}			

		// 마우스 위치 기반 줌
		FVector2D MousePosition;

		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			if (UGameViewportClient* ViewportClient = PlayerController->GetWorld()->GetGameViewport())
			{
				// Get mouse position in the viewport
				ViewportClient->GetMousePosition(MousePosition);
			}
		}

		FVector2D ViewportSize = FVector2D::ZeroVector;
		if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
		{
			ViewportClient->GetViewportSize(ViewportSize);
		}

		// 게임 뷰포트의 중앙을 계산
		FVector2D ViewportCenter = ViewportSize * 0.5f;

		// 뷰포트 중앙을 기준으로 오차 계산
		FVector2D OffsetFromCenter = MousePosition - ViewportCenter;
		bIsActor ? AccumulatedInputDelta.Y += (OffsetFromCenter.Y * 0.1f) : AccumulatedInputDelta.Y -= (OffsetFromCenter.Y * 0.1f);
	}
}

void UInputInteractor::AdjustVerticalBounds()
{
	FinalTranslationMinConstraints = FVector2D(InitialTargetsTransform.GetLocation()) + InteractionData.TranslationMinPadding;
	FinalTranslationMaxConstraints = FVector2D(InitialTargetsTransform.GetLocation()) + InteractionData.TranslationMaxPadding;

	if (InteractionData.bUseZoomConstraint)
	{
		float VisibleHeight = CalculateVisibleHeight();
		if (InteractionData.SceneViewHeight > VisibleHeight)
		{
			// 오차를 계산하여 추가
			float HalfInvisibleSize = (InteractionData.SceneViewHeight - VisibleHeight) * 0.5f;

			FinalTranslationMinConstraints.Y -= HalfInvisibleSize;
			FinalTranslationMaxConstraints.Y += HalfInvisibleSize;
		}
	}

}

float UInputInteractor::CalculateVisibleHeight()
{
	USceneComponent* ZoomControlTarget = IInterfaceInputInteractor::Execute_GetZoomControlTarget(GetOuter());
	USceneComponent* ZoomLookTarget = IInterfaceInputInteractor::Execute_GetZoomLookTarget(GetOuter());

	if (ZoomControlTarget == nullptr || ZoomLookTarget == nullptr)
	{
		return 0.f;
	}

	float Distance = FVector::Dist(ZoomControlTarget->GetComponentLocation(), ZoomLookTarget->GetComponentLocation());

	// 카메라의 절반 FOV와 현재 거리를 사용하여 보이는 높이를 계산
	float HalfFOV = FMath::DegreesToRadians(IInterfaceInputInteractor::Execute_GetCameraFOV(GetOuter()) / 2.0f);

	// 현재 거리에서 보이는 높이 계산
	return Distance * FMath::Tan(HalfFOV);
}



void UInputInteractor::OnInputMoved(FVector2D DeltaPosition)
{
	if (false == IsSetupComplete() || (InteractionData.AxisHandlingModeX == EAxisHandlingMode::None && InteractionData.AxisHandlingModeY == EAxisHandlingMode::None))
	{
		return;
	}

	FVector2D DeltaValue = bIsActor ? -1.f * DeltaPosition : DeltaPosition;
	FVector2D RotateDalta = FVector2D::ZeroVector;
	FVector2D TranslateDelta = FVector2D::ZeroVector;

	switch (InteractionData.AxisHandlingModeX)
	{
		case EAxisHandlingMode::Translate: TranslateDelta.X = DeltaValue.X; break;
		case EAxisHandlingMode::Rotate: RotateDalta.X = DeltaValue.X; break;
	}

	switch (InteractionData.AxisHandlingModeY)
	{
		case EAxisHandlingMode::Translate:TranslateDelta.Y = DeltaValue.Y; break;
		case EAxisHandlingMode::Rotate: RotateDalta.Y = DeltaValue.Y; break;
	}

	RotateDalta *= InteractionData.RotationDeltaMultiplier;
	TranslateDelta *= InteractionData.TranslationDeltaMultipliers;

	if (false == RotateDalta.IsZero())
	{
		if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetRotationTarget(GetOuter()))
		{
			ApplyRotationConstraints(RotateDalta);
			Target->AddWorldRotation(FRotator(RotateDalta.Y, RotateDalta.X, 0.f));
		}
	}

	if (false == TranslateDelta.IsZero())
	{
		if (USceneComponent* Target = IInterfaceInputInteractor::Execute_GetTranslationTarget(GetOuter()))
		{
			FVector2D CalcTranslate = FVector2D(Target->GetComponentLocation().Y + TranslateDelta.X, Target->GetComponentLocation().Z + TranslateDelta.Y);
			ApplyTranslationConstraints(CalcTranslate);
			Target->SetWorldLocation(FVector(Target->GetComponentLocation().X, CalcTranslate.X, CalcTranslate.Y));
		}
	}
}

void UInputInteractor::OnInputWheelEvent(float Delta)
{
	if (false == IsSetupComplete() || false == InteractionData.bEnableZoom)
	{
		return;
	}

	UpdateZoomStep((int)Delta);
}


bool UInputInteractor::IsTickable() const
{
	if (!IsValid(this))
	{
		return false;
	}

	if (HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
	{
		return false;
	}		

	return _IsTickable && !IsTemplate();
}
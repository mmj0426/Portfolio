#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SceneActor/InterfaceInputInteractor.h"
#include "SceneActor/InputInteractor.h"
//#include "ATPCInterpolationSpeed.h"
#include "BaseSceneActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateModelFinished);

UCLASS(BlueprintType)
class PORTFOLIO_API ABaseSceneActor : public AActor, public IInterfaceInputInteractor
{
	GENERATED_BODY()
	
public:	
	ABaseSceneActor();

public:
	static ABaseSceneActor* CurrentActiveSceneActor;
	static void SetActiveSceneActor(ABaseSceneActor* ActiveSceneActor);

	ABaseSceneActor* PrevActiveSceneActor;
	virtual void Activate();
	virtual void Deactivate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitInputInteractor();
	virtual void InitInputInteractor_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCameraFOV(float Value);
	virtual void SetCameraFOV_Implementation(float Value);

	// 플레이어 카메라로 초기화
	void ResetCameraToPlayerView();

	UPROPERTY() float PlayerCameraFOV = 0.f;
	UPROPERTY() UCurveFloat* PlayerCurveFloat = NULL;

	// Input Bind Function
	UFUNCTION() virtual void OnScreenDrag(FVector Location, FVector2D DeltaValue);
	UFUNCTION() virtual void OnScreenZoom(float DeltaValue);
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FInputInteractionData InteractionData;

	// SubTalk Visibility
	UFUNCTION() void SetSubTalkVisibility(bool CanShow);

public:
	float InitFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName LitPresetID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class USceneComponent> DefaultScene;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UCameraComponent> CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UChildActorComponent> LightComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UInputInteractor> InputInteractor;

#pragma region InterfaceInputInteraction
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	class USceneComponent* GetZoomControlTarget() const;
	virtual class USceneComponent* GetZoomControlTarget_Implementation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	class USceneComponent* GetTranslationTarget() const;
	virtual class USceneComponent* GetTranslationTarget_Implementation() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	float GetCameraFOV() const;
	float GetCameraFOV_Implementation() const;

#pragma endregion

#pragma region Light
	void ApplySceneActorBGLight(FName ID);
#pragma endregion

#pragma region Delegate
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, VisibleAnywhere)
	FOnCreateModelFinished CreateModelSceneFinished;
#pragma endregion
};

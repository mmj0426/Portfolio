#pragma once

#include "CoreMinimal.h"
#include "UObject/interface.h"
#include "InterfaceInputInteractor.generated.h"

UINTERFACE(MinimalAPI)
class UInterfaceInputInteractor : public UInterface
{
    GENERATED_BODY()
};

class PORTFOLIO_API IInterfaceInputInteractor
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	class USceneComponent* GetRotationTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	class USceneComponent* GetTranslationTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	class USceneComponent* GetZoomControlTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	class USceneComponent* GetZoomLookTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = InputInteraction)
	float GetCameraFOV() const;
};

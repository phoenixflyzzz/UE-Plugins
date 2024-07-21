#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FeetIK.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ADJUSTIK_API UFeetIK : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFeetIK();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AdjustIK();
	std::tuple<float, FRotator> TraceIK(FName InSocketName, float HalfHeight);

public:
	UPROPERTY()
	class ACharacter* Character;

	float CapsuleHalfHeight;
	float CapsuleRadius;

	UPROPERTY(EditAnywhere, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	float TraceDistance = 60.f;

	UPROPERTY(EditAnywhere, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	float SCaleOffset = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	FVector FootEffectL;

	UPROPERTY(BlueprintReadOnly, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	FVector FootEffectR;

	UPROPERTY(BlueprintReadOnly, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	FVector PelvisOffset;

	UPROPERTY(BlueprintReadOnly, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	FRotator FootRotatorL;

	UPROPERTY(BlueprintReadOnly, Category = "FeetIK", meta = (AllowPrivateAccess = "true"))
	FRotator FootRotatorR;

};

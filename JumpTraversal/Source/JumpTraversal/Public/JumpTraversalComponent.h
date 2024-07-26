#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "JumpTraversalComponent.generated.h"

UENUM(BlueprintType)
enum class EJumpType : uint8
{
	EJT_Initial UMETA(DisplayName = "Initial"),
	EJT_Jump UMETA(DisplayName = "Jump"),
	EJT_Vault UMETA(DisplayName = "Vault"),
	EJT_Mantle UMETA(DisplayName = "Mantle"),
	EJT_Climb UMETA(DisplayName = "Climb"),

	EJT_MAX UMETA(DisplayName = "MAX")
};

USTRUCT(BlueprintType)
struct FJumpTraceRes
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EJumpType JumpType;

	UPROPERTY(BlueprintReadOnly)
	float WallHeight;

	FJumpTraceRes()
	{}

	FJumpTraceRes(EJumpType JT, float WH) :
		JumpType(JT), WallHeight(WH)
	{}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JUMPTRAVERSAL_API UJumpTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

	EJumpType JumpType = EJumpType::EJT_Jump;

public:	
	UJumpTraversalComponent();

	UFUNCTION(BlueprintCallable)
	FJumpTraceRes GetJumpType();


protected:
	virtual void BeginPlay() override;

	float JumpTypeJudge();
	bool ObliquityTrace();
	std::pair<float, float> WallTrace(bool bObliquityTrace);
	float WallWidthTrace(FVector TraceStart, FHitResult LastOutHit);
	bool WallThinknessTrace(FVector TraceStart, FVector ActorForward);

private:
	UPROPERTY()
	class ACharacter* Character;

	float CapsuleHalfHeight;
	FVector ActorLoc;
	FVector ActorFwd;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceDistance = 100.f;

	UPROPERTY(EditAnywhere, Category = "Trace")
	int32 MaxTraceNum = 20;

	UPROPERTY(EditAnywhere, Category = "Traversal")
	float MaxClimbHeight = 400.f;

	UPROPERTY(EditAnywhere, Category = "Traversal")
	float DistanceOffset = 40.f;

	UPROPERTY(EditAnywhere, Category = "Traversal")
	float Thinkness = 30.f;

public:
	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* EquippedVaultMontage;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* UnEquippedVaultMontage;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* EquippedMantleMontage;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* UnEquippedMantleMontage;

	UPROPERTY(EditAnywhere, Category = "Montage")
	UAnimMontage* ClimbMontage;
};

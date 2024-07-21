#include "FeetIK.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UFeetIK::UFeetIK()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UFeetIK::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		CapsuleHalfHeightCrouched = Character->GetCharacterMovement()->CrouchedHalfHeight;
	}
}


void UFeetIK::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AdjustIK();
}

void UFeetIK::AdjustIK()
{
	float HalfHeight = Character->bIsCrouched ? CapsuleHalfHeightCrouched : CapsuleHalfHeight;
	Character->GetCapsuleComponent()->SetCapsuleHalfHeight(HalfHeight);

	auto [OffsetL, RotationL] = TraceIK(FName("foot_l"), HalfHeight);
	auto [OffsetR, RotationR] = TraceIK(FName("foot_r"), HalfHeight);

	float TraceHipOffset = FMath::Min<float>(OffsetL, OffsetR);

	if (!Character->bIsCrouched)
	{
		PelvisOffset.Z = FMath::FInterpTo(PelvisOffset.Z, TraceHipOffset, GetWorld()->GetDeltaSeconds(), 5.f);
	}
	else
	{
		if (TraceHipOffset > -10.f && Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		{
			PelvisOffset.Z = FMath::FInterpTo(PelvisOffset.Z, TraceHipOffset, GetWorld()->GetDeltaSeconds(), 5.f);
		}
		else if (TraceHipOffset > -20.f)
		{
			PelvisOffset.Z = FMath::FInterpTo(PelvisOffset.Z, TraceHipOffset + 10.f, GetWorld()->GetDeltaSeconds(), 5.f);
		}
		else if (TraceHipOffset > -30.f)
		{
			PelvisOffset.Z = FMath::FInterpTo(PelvisOffset.Z, TraceHipOffset + 20.f, GetWorld()->GetDeltaSeconds(), 5.f);
		}
		else if (TraceHipOffset > -40.f)
		{
			PelvisOffset.Z = FMath::FInterpTo(PelvisOffset.Z, TraceHipOffset + 30.f, GetWorld()->GetDeltaSeconds(), 5.f);
		}
		else
		{
			PelvisOffset.Z = FMath::FInterpTo(PelvisOffset.Z, TraceHipOffset + 50.f, GetWorld()->GetDeltaSeconds(), 5.f);
		}
	}

	FootEffectL.X = FMath::FInterpTo(FootEffectL.X, OffsetL - PelvisOffset.Z, GetWorld()->GetDeltaSeconds(), 15.f);
	FootEffectR.X = FMath::FInterpTo(FootEffectR.X, -(OffsetR - PelvisOffset.Z), GetWorld()->GetDeltaSeconds(), 15.f);

	FootRotatorL.Roll = FMath::FInterpTo(FootRotatorL.Roll, RotationL.Roll, GetWorld()->GetDeltaSeconds(), 15.f);
	FootRotatorL.Pitch = FMath::FInterpTo(FootRotatorL.Pitch, RotationL.Pitch, GetWorld()->GetDeltaSeconds(), 15.f);
	FootRotatorR.Roll = FMath::FInterpTo(FootRotatorR.Roll, RotationR.Roll, GetWorld()->GetDeltaSeconds(), 15.f);
	FootRotatorR.Pitch = FMath::FInterpTo(FootRotatorR.Pitch, RotationR.Pitch, GetWorld()->GetDeltaSeconds(), 15.f);

	Character->GetCapsuleComponent()->SetCapsuleHalfHeight(HalfHeight + PelvisOffset.Z / 2.f);
}

std::tuple<float, FRotator> UFeetIK::TraceIK(FName InSocketName, float HalfHeight)
{
	if (!Character)
	{
		Character = Cast<ACharacter>(GetOwner());
		if (!Character) return std::tuple<float, FRotator>();
	}

	const FVector SocketLoc = Character->GetMesh()->GetSocketLocation(InSocketName);
	const FVector ActorLoc = Character->GetActorLocation();

	const FVector TraceStart = FVector(SocketLoc.X, SocketLoc.Y, ActorLoc.Z - HalfHeight / 2);
	const FVector TraceEnd = FVector(SocketLoc.X, SocketLoc.Y, ActorLoc.Z - HalfHeight - TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams(TEXT("QueryParams"), true, nullptr);
	CollisionQueryParams.bTraceComplex = true;
	CollisionQueryParams.bReturnPhysicalMaterial = false;
	CollisionQueryParams.AddIgnoredActor(Character);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionQueryParams);
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.5f);

	float TraceOffset = bHit ? HalfHeight / 2 - HitResult.Distance + SCaleOffset : 0.f;	// ¸ºÖµ

	FRotator TraceRotation;
	TraceRotation.Roll = FMath::Atan2(HitResult.Normal.Y, HitResult.Normal.Z) * 50.f;
	TraceRotation.Pitch = FMath::Atan2(HitResult.Normal.X, HitResult.Normal.Z) * (-50.f);

	return std::make_tuple(TraceOffset, TraceRotation);
}



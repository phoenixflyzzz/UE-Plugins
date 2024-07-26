#include "JumpTraversalComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UJumpTraversalComponent::UJumpTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UJumpTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
}

FJumpTraceRes UJumpTraversalComponent::GetJumpType()
{
	JumpType = EJumpType::EJT_Jump;
	float WallHeight = JumpTypeJudge();
	return FJumpTraceRes(JumpType, WallHeight);
}

float UJumpTraversalComponent::JumpTypeJudge()
{
	if (!Character)
	{
		Character = Cast<ACharacter>(GetOwner());
		if (!Character) return 0.f;
	}

	ActorLoc = Character->GetActorLocation();
	ActorFwd = Character->GetActorForwardVector();

	bool bObliquityTrace = ObliquityTrace();
	auto [WallHeight, WallDistance] = WallTrace(bObliquityTrace);

	if (JumpType != EJumpType::EJT_Jump)
	{
		FVector TargetLoc = ActorLoc + ActorFwd * (WallDistance - DistanceOffset);
		Character->GetCapsuleComponent()->SetWorldLocation(TargetLoc);
	}

	return WallHeight;
}

bool UJumpTraversalComponent::ObliquityTrace()
{
	FVector TraceStart = ActorLoc + ActorFwd * 20.f - FVector(0.f, 0.f, CapsuleHalfHeight / 2);
	FVector TraceEnd = TraceStart + ActorFwd * TraceDistance + FVector(0.f, 0.f, 60.f);

	// Ð±Ãæ¼ì²â
	FHitResult OutHit;
	bool bIsTraceHit = UKismetSystemLibrary::LineTraceSingle(
		Character->GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		OutHit,
		true
	);

	return bIsTraceHit;
}

std::pair<float, float> UJumpTraversalComponent::WallTrace(bool bObliquityTrace)
{
	float WallHeight;
	float WallDistance;

	bool bLastTraceHit = false;
	FHitResult LastOutHit;
	int32 StartTraceNum = bObliquityTrace ? 0 : 3;
	for (int32 i = StartTraceNum; i < MaxTraceNum; ++i)
	{
		FVector TraceStart = ActorLoc + ActorFwd * 20.f + FVector(0.f, 0.f, i * 20.f - CapsuleHalfHeight / 2);
		FVector TraceEnd = TraceStart + ActorFwd * TraceDistance;
		FHitResult OutHit;
		bool bTraceHit = UKismetSystemLibrary::BoxTraceSingle(
			Character->GetWorld(),
			TraceStart,
			TraceEnd,
			FVector(0.f, 10.f, 10.f),
			Character->GetActorRotation(),
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::None,
			OutHit,
			true
		);

		// ¼ÆËã½ÇÉ«ÓëÇ½µÄ¼Ð½Ç
		if (bTraceHit)
		{
			FVector Vec1 = FVector(OutHit.Normal.X, OutHit.Normal.Y, 0.f);
			FVector Vec2 = FVector(ActorFwd.X, ActorFwd.Y, 0.f);
			float Angle = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Vector_CosineAngle2D(Vec1, Vec2));

			if (Angle < 150.f)
			{
				JumpType = EJumpType::EJT_Jump;
				return std::make_pair(WallHeight, WallDistance);
			}
		}

		if (bLastTraceHit && !bTraceHit)
		{
			WallDistance = LastOutHit.Distance;

			// ¼ì²âÏÂÇ½
			TraceStart = LastOutHit.Location + ActorFwd * 10.f + FVector(0.f, 0.f, 20.f);

			TraceEnd = TraceStart - FVector(0.f, 0.f, TraceDistance);
			FHitResult OutHitDown;
			bool bLineTraceDown = UKismetSystemLibrary::LineTraceSingle(
				Character->GetWorld(),
				TraceStart,
				TraceEnd,
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
				false,
				TArray<AActor*>(),
				EDrawDebugTrace::None,
				OutHitDown,
				true,
				FLinearColor::Blue,
				FLinearColor::Yellow
			);

			WallHeight = TraceStart.Z - OutHitDown.Distance - ActorLoc.Z + CapsuleHalfHeight;

			if (WallHeight > MaxClimbHeight)
			{
				// Ç½Ì«¸ß
				JumpType = EJumpType::EJT_Jump;
				return std::make_pair(WallHeight, WallDistance);
			}

			// ¼ì²âÉÏÇ½
			TraceEnd = TraceStart + FVector(0.f, 0.f, CapsuleHalfHeight * 2);
			FHitResult OutHitUp;
			bool bLineTraceUp = UKismetSystemLibrary::LineTraceSingle(
				Character->GetWorld(),
				TraceStart,
				TraceEnd,
				UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
				false,
				TArray<AActor*>(),
				EDrawDebugTrace::None,
				OutHitUp,
				true,
				FLinearColor::Black,
				FLinearColor::Yellow
			);

			float WallGap = bLineTraceUp ? OutHitUp.Distance + OutHitDown.Distance : 1000.f;
			float CrouchedHalfHeight = Character->GetCharacterMovement()->CrouchedHalfHeight;
			if (!bLineTraceUp || WallGap > CrouchedHalfHeight * 2)
			{
				// ÉÏÇ½²»´æÔÚ»òÉÏÏÂÇ½¾àÀë³¬¹ý¶×·ü¸ß¶È
			
				// ¼ì²éÇ½¿í
				float WallWidth = WallWidthTrace(TraceStart, LastOutHit);
				if (WallWidth < Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 2)
				{
					// Ç½Ì«Õ­
					JumpType = EJumpType::EJT_Jump;
					continue;
				}

				// ¼ì²éÇ½ºñ
				bool bTraceThinkness = WallThinknessTrace(TraceStart, ActorFwd);

				if (WallHeight > CapsuleHalfHeight * 3)
				{
					JumpType = EJumpType::EJT_Climb;
				}
				else
				{
					if (bTraceThinkness)
					{
						JumpType = EJumpType::EJT_Mantle;
						if (WallGap < CapsuleHalfHeight * 2)
						{
							Character->Crouch();
						}
					}
					else
					{
						JumpType = EJumpType::EJT_Vault;
					}
				}

				break;
			}
			else
			{
				continue;
			}
			
		}

		bLastTraceHit = bTraceHit;
		LastOutHit = OutHit;
	}

	return std::make_pair(WallHeight, WallDistance);;
}

float UJumpTraversalComponent::WallWidthTrace(FVector TraceStart, FHitResult LastOutHit)
{
	FVector TraceWidthStart = TraceStart;
	FVector TraceWidthEnd;

	float LeftLength = 1000.f;
	float RightLength = 1000.f;
	for (int i = 0; i < 3; ++i, TraceWidthStart.Z += 10)
	{
		FVector WallLeftVec = LastOutHit.Normal;
		WallLeftVec.Z = 0.f;
		FRotator Rotation(0.f, 90.f, 0.f);
		WallLeftVec = Rotation.RotateVector(WallLeftVec);

		TraceWidthEnd = TraceWidthStart + WallLeftVec * 200.f;
		FHitResult OutHitLeft;
		bool bLineTraceLeft = UKismetSystemLibrary::LineTraceSingle(
			Character->GetWorld(),
			TraceWidthStart,
			TraceWidthEnd,
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::None,
			OutHitLeft,
			true,
			FLinearColor::Black,
			FLinearColor::Yellow
		);

		FVector WallRightVec = LastOutHit.Normal;
		WallRightVec.Z = 0.f;
		Rotation.Yaw = -90.f;
		WallRightVec = Rotation.RotateVector(WallRightVec);

		TraceWidthEnd = TraceWidthStart + WallRightVec * 200.f;
		FHitResult OutHitRight;
		bool bLineTraceRight = UKismetSystemLibrary::LineTraceSingle(
			Character->GetWorld(),
			TraceWidthStart,
			TraceWidthEnd,
			UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			false,
			TArray<AActor*>(),
			EDrawDebugTrace::None,
			OutHitRight,
			true,
			FLinearColor::Blue,
			FLinearColor::Yellow
		);

		LeftLength = bLineTraceLeft ? FMath::Min(LeftLength, OutHitLeft.Distance) : LeftLength;
		RightLength = bLineTraceRight ? FMath::Min(RightLength, OutHitRight.Distance) : RightLength;

		if (LeftLength < CapsuleHalfHeight / 4 || RightLength < CapsuleHalfHeight / 4)
		{
			return 0.f;
		}
	}

	return LeftLength + RightLength;
}

bool UJumpTraversalComponent::WallThinknessTrace(FVector TraceStart, FVector ActorForward)
{
	TraceStart += ActorForward * Thinkness;
	FVector TraceEnd = TraceStart - FVector(0.f, 0.f, CapsuleHalfHeight);
	FHitResult OutHitThinkness;
	bool bTraceThinkness = UKismetSystemLibrary::LineTraceSingle(
		Character->GetWorld(),
		TraceStart,
		TraceEnd,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::None,
		OutHitThinkness,
		true,
		FLinearColor::Red,
		FLinearColor::Yellow
	);

	return bTraceThinkness;
}





// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss1AnimInstance.h"

#include "Boss1Character.h"
#include "Components/AudioComponent.h"

UBoss1AnimInstance::UBoss1AnimInstance()
{
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Attack(TEXT("AnimMontage'/Game/ThirdPerson/Blueprints/Monster/Boss1/Anim/Boss1_Attack.Boss1_Attack'"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Gimmick1(TEXT("AnimMontage'/Game/ThirdPerson/Blueprints/Monster/Boss1/Anim/Boss1_Gimmick1.Boss1_Gimmick1'"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Gimmick2(TEXT("AnimMontage'/Game/ThirdPerson/Blueprints/Monster/Boss1/Anim/Boss1_Gimmick2.Boss1_Gimmick2'"));
	
	if(Attack.Succeeded())
		_attackMontage=Attack.Object;
	if(Gimmick1.Succeeded())
		_gimmick1Montage=Gimmick1.Object;
	if(Gimmick2.Succeeded())
		_gimmick2Montage=Gimmick2.Object;
}

void UBoss1AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if(IsValid(Pawn))
	{
		_speed=Pawn->GetVelocity().Size();

		ABoss1Character* Character = Cast<ABoss1Character>(Pawn);
		if(Character)
		{
			// TODO
		}
	}
}

void UBoss1AnimInstance::PlayAttackMontage()
{
	Montage_Play(_attackMontage, 1.f);
}

void UBoss1AnimInstance::JumpToSection()
{
	// 랜덤으로 기본 공격
	int32 RandomIndex = FMath::RandRange(0,2);
	FName Name = FName(*FString::Printf(TEXT("Attack%d"), RandomIndex));
	Montage_JumpToSection(Name, _attackMontage);
}

void UBoss1AnimInstance::PlayGimmick1Montage()
{
	Montage_Play(_gimmick1Montage, 1.f);
}

void UBoss1AnimInstance::AnimNotify_AttackHit()
{
	// TODO 검 휘두르는 소리 
}

void UBoss1AnimInstance::AnimNotify_Gimmick1()
{
	// TODO Homepos 문제 해결 후 위치를 Homepos로 변경
	TryGetPawnOwner()->SetActorLocation(FVector(700.0f, 400.0f, 226.0f));
}

void UBoss1AnimInstance::AnimNotify_ShieldHit()
{
	// TODO Gimmick2 방패 치는 소리
}

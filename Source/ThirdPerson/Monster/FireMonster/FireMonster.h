// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FireMonsterAnim.h"
#include "../Monster.h"
#include "ThirdPerson/Monster/DarknessMonster/DarknessMonsterAnim.h"
#include "FireMonster.generated.h"

/**
 * 
 */
UCLASS()
class THIRDPERSON_API AFireMonster : public AMonster
{
	GENERATED_BODY()

public:
	AFireMonster();

	virtual void BeginPlay() override;
	virtual void Attack() override;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	UPROPERTY()
	UFireMonsterAnim* AnimInstance;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCharacter.h"
#include "MakePlayerController.h"
#include "CountessAnimInstance.h"
#include "AI/NavigationSystemBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"

AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	//캐릭터 불러오기
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SM(TEXT("SkeletalMesh'/Game/ParagonCountess_change/Characters/Heroes/Countess/Meshes/SM_Countess.SM_Countess'"));
	if (SM.Succeeded())
	{
		GetMesh()-> SetSkeletalMesh(SM.Object);
	}
	
	//애니메이션 블루프린트 로딩 및 세팅
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	
	static ConstructorHelpers::FClassFinder<UAnimInstance>ABP(TEXT("AnimBlueprint'/Game/ThirdPerson/Blueprints/Character/ABP_MyAnime'_C"));// 2를 붙여서 실험
	if (ABP.Succeeded())
	{ 
		GetMesh()->SetAnimClass(ABP.Class);
	}


	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f,0.f,-88.f), FRotator(0.f, -90.f, 0.f));//yaw-90
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	GetCharacterMovement()->bOrientRotationToMovement = false;//tr
	GetCharacterMovement()->bConstrainToPlane = false;
	 GetCharacterMovement()->bSnapToPlaneAtStart = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;//false
	bUseControllerRotationRoll = false;//fa
	

	//스프링암 설치 및 설정
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // 
	CameraBoom->TargetArmLength= 400.f;
	CameraBoom->SetRelativeRotation(FRotator(-30.f, -90.f, 0.f));
	CameraBoom->bDoCollisionTest= false;
	CameraBoom->bUsePawnControlRotation=true;//

	//카메라 설치 
	TopDownCameraComponent= CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = true;//tr

	MotionWarpComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Warp"));
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// MyCharacter내에서 캐릭터 컨트롤러, 애니인스턴스 사용할것 설정
	MyControler = Cast<AMakePlayerController>(GetController());
	AnimInstance = Cast<UCountessAnimInstance>(GetMesh()->GetAnimInstance());
	//델리게이트 생성 
	AnimInstance->OnMontageEnded.AddDynamic(this, &AMyCharacter::OnAttackMontageEnded);
	AnimInstance->OnMontageStarted.AddDynamic(this, &AMyCharacter::OnAttackMontageStarted);
	
}

void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}



// Called every frame 캐릭터 틱-함수, 스킬 T에 사용 
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// 바인딩 T델리게이트로 열거형 스킬 불러오기 
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("LeftClick"),EInputEvent::IE_Pressed,this,&AMyCharacter::Attack);
	PlayerInputComponent->BindAction(TEXT("RightClick"),EInputEvent::IE_Pressed,this,&AMyCharacter::Parring);
	PlayerInputComponent->BindAction(TEXT("Shift"),EInputEvent::IE_Pressed,this,&AMyCharacter::Shift);
	PlayerInputComponent->BindAction(TEXT("Shift"),EInputEvent::IE_Released,this,&AMyCharacter::ShiftReleasd);
	
	InputComponent->BindAction<TDelegate<void(ECharacterState)>>("QSkill", IE_Pressed, this, &AMyCharacter::SkillPress, ECharacterState::State_SkillQ);
	InputComponent->BindAction<TDelegate<void(ECharacterState)>>("ESkill", IE_Pressed, this, &AMyCharacter::SkillPress, ECharacterState::State_SkillE);
	InputComponent->BindAction<TDelegate<void(ECharacterState)>>("FSkill", IE_Pressed, this, &AMyCharacter::SkillPress, ECharacterState::State_SkillF);
	InputComponent->BindAction<TDelegate<void(ECharacterState)>>("RSkill", IE_Pressed, this, &AMyCharacter::SkillPress, ECharacterState::State_SkillR);
	InputComponent->BindAction<TDelegate<void(ECharacterState)>>("TSkill", IE_Pressed, this, &AMyCharacter::SkillPress, ECharacterState::State_SkillT);

	PlayerInputComponent->BindAxis(TEXT("UpDown"),this,&AMyCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"),this,&AMyCharacter::LeftRight);
	
	PlayerInputComponent->BindAxis(TEXT("Turn Right"),this,&AMyCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis(TEXT("Look Up"),this,&AMyCharacter::AddControllerPitchInput);

}

bool AMyCharacter::GetAttacking()
{
	return IsAttacking;
}



void AMyCharacter::Shift()
{
	if(IsAttacking == true)
		return;
	// if(MyControler->WasInputKeyJustPressed(EKeys::LeftShift))
	// {
	// 	GetCharacterMovement()->MaxWalkSpeed=5000.f;
	// }

	float DeltaTime = FApp::GetDeltaTime();
	float ZoomFactor = (DeltaTime / 0.5f);
	ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);


	float A = 700.0f;
	float B = 200.0f;
	float lerp = UKismetMathLibrary::Lerp(A,B,ZoomFactor);
	CameraBoom->TargetArmLength= lerp;
	GetCharacterMovement()->MaxWalkSpeed= 5000.f;
	
}

void AMyCharacter::ShiftReleasd()
{
	CameraBoom->TargetArmLength= 400.f;
	GetCharacterMovement()->MaxWalkSpeed= 1000.f;
}

void AMyCharacter::UpDown(float Value)
{
	if(Value == 0.f || IsAttacking == true)
		return ;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	SetActorRotation(YawRotation);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void AMyCharacter::LeftRight(float Value)
{
	if(Value == 0.f || IsAttacking == true)
		return ;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	//const FRotator YawTurnRotation(0,(Rotation.Yaw)+90,0);
	//FRotator YawTurnRotation= YawTurnRotation = FRotator(0,(Rotation.Yaw)+(Value*90),0);
	
	
	// get right vector 
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //GetUnitAxis는 GetScaledAxis의 결과를 단위벡터로 변환하여 반환해주는 함수
	// add movement in that direction
	//SetActorRotation(YawTurnRotation);
	AddMovementInput(Direction, Value);
	
}

void AMyCharacter::Yaw(float Value)
{
	AddControllerYawInput(Value);
	UE_LOG(LogTemp,Warning,TEXT("회전을 실행하는 중입니다. "));
}

// 캐릭터 우클릭 평타 몬티지 호출 
void AMyCharacter::Attack()
{
	if(IsAttacking)
		return;
	IsAttacking = true;
	
	AnimInstance->PlayAttackMontage();

	AnimInstance -> JumpToSection(AttackIndex);
	AttackIndex = (AttackIndex +1) % 2;
	
}

void AMyCharacter::Parring()
{
	if(IsAttacking)
		return;
	IsAttacking = true;

	AnimInstance->PlayParringMontage();
	
}



void AMyCharacter::SkillPress(ECharacterState InputSkill)
{
	//중복 체크 
	if(IsAttacking)
		return;
	IsAttacking = true;
	// 열거형 저장
	
	MyCharacterState = InputSkill;

	
	// FVector FowardDistance= (this->GetActorForwardVector()*10000);
	// const FVector MovementInputVector = GetLastMovementInputVector();
	// if (MovementInputVector.IsZero())
	// {
	// 	MotionWarpComp->RemoveWarpTarget(TEXT("Warp"));
	// 	return;
	// }
	// const FVector PlayerLoc = GetActorLocation();
	// const FVector TargetLoc = (MovementInputVector.GetSafeNormal() * FowardDistance) + PlayerLoc;
	// //const FVector TargetLoc (0,0,0);
	// const FRotator TargetRotator = UKismetMathLibrary::MakeRotFromX(MovementInputVector);
	// MotionWarpComp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("Warp"), TargetLoc, TargetRotator);
	
	switch(InputSkill)
	{
	case ECharacterState::State_SkillQ:
		UE_LOG(LogTemp,Warning,TEXT("Q스킬발동"));
		AnimInstance->PlaySkillQ();

		AnimInstance -> JumpToSectionQ(AttackIndexQ);
		AttackIndexQ = (AttackIndexQ +1) % 4;
		break;
		
	case ECharacterState::State_SkillE:
		UE_LOG(LogTemp,Warning,TEXT("E스킬발동"));
		AnimInstance->PlaySkillE();

		AnimInstance -> JumpToSectionE(AttackIndexE);
		AttackIndexE = (AttackIndexE +1) % 4;
		break;
		
	case ECharacterState::State_SkillF:
		
		UE_LOG(LogTemp,Warning,TEXT("E스킬발동"));
		AnimInstance->PlaySkillF();
		break;
		
	case ECharacterState::State_SkillR:
		UE_LOG(LogTemp,Warning,TEXT("R스킬발동"));
		AnimInstance->PlaySkillSequence();
		// SkillVector = (-1 *(this->GetActorForwardVector())*5000);
		// this->LaunchCharacter(SkillVector,true,true);
		vecMove = ((this->GetActorForwardVector())*-10000);
		this->GetMovementComponent()->Velocity = vecMove;
		this->AddMovementInput(vecMove);
		break;
		
	case ECharacterState::State_SkillT:
		
		skill_Length = 2000; // 스킬 범위 
	
		UE_LOG(LogTemp,Warning,TEXT("T분기점2"));
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		SetActorRotation(YawRotation);

		const FVector lotation = this->GetActorLocation();
	
		
		SkillVector = ((this->GetActorForwardVector())*skill_Length);
		
		FVector Location(SkillVector.X+lotation.X,SkillVector.Y+lotation.Y,lotation.Z+1000);
		
		float TraceDistance = 2000.0f; 
		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location - FVector(0, 0, TraceDistance), ECC_Visibility))
		{
			FVector AdjustedLocation = HitResult.ImpactPoint + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+100);
			SetActorLocation(AdjustedLocation, true, &HitResult);
		}

		// vecMove = ((this->GetActorForwardVector())*50000);
		// this->GetMovementComponent()->Velocity = vecMove;
		// this->AddMovementInput(vecMove);

		
		// get forward vector
		//SetActorLocation(Location);
		
		AnimInstance->PlaySkillT();
		break;
	}
}

void AMyCharacter::OnAttackMontageStarted(UAnimMontage* Montage)
{
	UE_LOG(LogTemp,Warning,TEXT("몽티지시작"));
	FNavigationSystem::StopMovement(*GetController());

	//이동모션 변경 스위치 
	AnimInstance->ChangeAttacked();
}


void AMyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp,Warning,TEXT("몽티지 종료"));
	//몽티지 중복처리 
	IsAttacking = false;
	AnimInstance->ChangeAttacked();
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MetaBallsController.generated.h"

UENUM(BlueprintType)
enum class EAdjustmentState : uint8
{
	ZOOM,
	CENTER_RADIUS,
	ORBIT_RADIUS,
	DISTANCE_FROM_CENTER,
	NUM_SPHERES,
	ORBITS_PER_SECOND,
	ORBIT_SPEED_OFFSET,
	ORBIT_BEGIN_OFFSET,
	RIPPLE
};

UCLASS()
class MSGDEMO_API AMetaBallsController : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMetaBallsController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void BeginIncreaseParam();
	void BeginDecreaseParam();
	void EndIncreaseParam();
	void EndDecreaseParam();

	void BeginAdjustCenterRadius();
	void BeginAdjustOrbitRadius();
	void BeginAdjustDistanceFromCenter();
	void BeginAdjustNumSpheres();
	void BeginAdjustOrbitsPerSecond();
	void BeginAdjustOrbitSpeedOffset();
	void BeginAdjustOrbitBeginOffset();
	void BeginAdjustRipple();

	void EndAdjustCenterRadius();
	void EndAdjustOrbitRadius();
	void EndAdjustDistanceFromCenter();
	void EndAdjustNumSpheres();
	void EndAdjustOrbitsPerSecond();
	void EndAdjustOrbitSpeedOffset();
	void EndAdjustOrbitBeginOffset();
	void EndAdjustRipple();

	void IncrementParam(float deltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EAdjustmentState m_state = EAdjustmentState::ZOOM;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float m_incrementDuration = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float m_lerpDuration = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_zoomMin = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_zoomMax = 1.75f;
	float m_incrementTimer = 0.0f;
	bool m_increasingParams = false;
	bool m_decreasingParams = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_centerRadius = 300.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_orbitRadius = 150.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_distanceFromCenter = 50.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_numSpheres = 10.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_orbitsPerSecond = 0.1f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_orbitSpeedOffset = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_orbitBeginOffset = 0.7f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_ripple = 3.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MetaBallParams")
	float m_zoom = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_centerRadiusIncrementAmount = 25.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_orbitRadiusIncrementAmount = 25.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_distanceFromCenterIncrementAmount = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_numSpheresIncrementAmount = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_orbitsPerSecondIncrementAmount = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_orbitSpeedOffsetIncrementAmount = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_orbitBeginOffsetIncrementAmount = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_rippleIncrementAmount = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IncrementAmount")
	float m_zoomIncrementAmount = 1.0f;

protected:

	float m_targetCenterRadius = 300.0f;
	float m_targetOrbitRadius = 150.0f;
	float m_targetDistanceFromCenter = 50.0f;
	float m_targetNumSpheres = 10.0f;
	float m_targetOrbitsPerSecond = 0.1f;
	float m_targetOrbitSpeedOffset = 0.5f;
	float m_targetOrbitBeginOffset = 0.7f;
	float m_targetRipple = 3.0f;
	float m_targetZoom = 1.0f;

	float m_initialCenterRadius = 300.0f;
	float m_initialOrbitRadius = 150.0f;
	float m_initialDistanceFromCenter = 50.0f;
	float m_initialNumSpheres = 10.0f;
	float m_initialOrbitsPerSecond = 0.1f;
	float m_initialOrbitSpeedOffset = 0.5f;
	float m_initialOrbitBeginOffset = 0.7f;
	float m_initialRipple = 3.0f;
	float m_initialZoom = 1.0f;

	float m_centerRadiusLerpTimer = m_lerpDuration;
	float m_orbitRadiusLerpTimer = m_lerpDuration;
	float m_distanceFromCenterLerpTimer = m_lerpDuration;
	//float m_numSpheresLerpTimer = m_lerpDuration;
	float m_orbitsPerSecondLerpTimer = m_lerpDuration;
	float m_orbitSpeedOffsetLerpTimer = m_lerpDuration;
	float m_orbitBeginOffsetLerpTimer = m_lerpDuration;
	float m_rippleLerpTimer = m_lerpDuration;
	float m_zoomLerpTimer = m_lerpDuration;
};

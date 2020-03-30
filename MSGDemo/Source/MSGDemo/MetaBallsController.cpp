// Fill out your copyright notice in the Description page of Project Settings.


#include "MetaBallsController.h"

// Sets default values
AMetaBallsController::AMetaBallsController()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMetaBallsController::BeginPlay()
{
	Super::BeginPlay();
	
	m_targetCenterRadius = m_centerRadius;
	m_targetOrbitRadius = m_orbitRadius;
	m_targetDistanceFromCenter = m_distanceFromCenter;
	m_targetNumSpheres = m_numSpheres;
	m_targetOrbitsPerSecond = m_orbitsPerSecond;
	m_targetOrbitSpeedOffset = m_orbitSpeedOffset;
	m_targetOrbitBeginOffset = m_orbitBeginOffset;
	m_targetRipple = m_ripple;
	m_targetZoom = m_zoom;

	m_initialCenterRadius = m_centerRadius;
	m_initialOrbitRadius = m_orbitRadius;
	m_initialDistanceFromCenter = m_distanceFromCenter;
	m_initialNumSpheres = m_numSpheres;
	m_initialOrbitsPerSecond = m_orbitsPerSecond;
	m_initialOrbitSpeedOffset = m_orbitSpeedOffset;
	m_initialOrbitBeginOffset = m_orbitBeginOffset;
	m_initialRipple = m_ripple;
	m_initialZoom = m_zoom;
}

// Called every frame
void AMetaBallsController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_incrementTimer += DeltaTime;
	if (m_incrementTimer >= m_incrementDuration)
	{
		float incrementMultiplier = 0.0f;
		if (m_increasingParams)
		{
			incrementMultiplier = 1.0f;
		}
		else if (m_decreasingParams)
		{
			incrementMultiplier = -1.0f;
		}
		IncrementParam(incrementMultiplier);
	}

	m_centerRadiusLerpTimer			= FMath::Min(m_lerpDuration, m_centerRadiusLerpTimer + DeltaTime);
	m_orbitRadiusLerpTimer			= FMath::Min(m_lerpDuration, m_orbitRadiusLerpTimer + DeltaTime);
	m_distanceFromCenterLerpTimer	= FMath::Min(m_lerpDuration, m_distanceFromCenterLerpTimer + DeltaTime);
	//m_numSpheresLerpTimer			= FMath::Min(m_lerpDuration, m_numSpheresLerpTimer + DeltaTime);
	m_orbitsPerSecondLerpTimer		= FMath::Min(m_lerpDuration, m_orbitsPerSecondLerpTimer + DeltaTime);
	m_orbitSpeedOffsetLerpTimer		= FMath::Min(m_lerpDuration, m_orbitSpeedOffsetLerpTimer + DeltaTime);
	m_orbitBeginOffsetLerpTimer		= FMath::Min(m_lerpDuration, m_orbitBeginOffsetLerpTimer + DeltaTime);
	m_rippleLerpTimer				= FMath::Min(m_lerpDuration, m_rippleLerpTimer + DeltaTime);
	m_zoomLerpTimer					= FMath::Min(m_lerpDuration, m_zoomLerpTimer + DeltaTime);

	m_centerRadius = FMath::Lerp(m_initialCenterRadius, m_targetCenterRadius, m_centerRadiusLerpTimer);
	m_orbitRadius = FMath::Lerp(m_initialOrbitRadius, m_targetOrbitRadius, m_orbitRadiusLerpTimer);
	m_distanceFromCenter = FMath::Lerp(m_initialDistanceFromCenter, m_targetDistanceFromCenter, m_distanceFromCenterLerpTimer);
	m_numSpheres = FMath::Lerp(m_initialNumSpheres, m_targetNumSpheres, 1.0f); // this is effectively an int so there's no need to lerp it
	m_orbitsPerSecond = FMath::Lerp(m_initialOrbitsPerSecond, m_targetOrbitsPerSecond, 1.0f); // lerping this value produces weird artifacts...
	m_orbitSpeedOffset = FMath::Lerp(m_initialOrbitSpeedOffset, m_targetOrbitSpeedOffset, 1.0f); // lerping this value produces weird artifacts...
	m_orbitBeginOffset = FMath::Lerp(m_initialOrbitBeginOffset, m_targetOrbitBeginOffset, 1.0f); // lerping this value produces weird artifacts...
	m_ripple = FMath::Lerp(m_initialRipple, m_targetRipple, m_rippleLerpTimer);
	m_zoom = FMath::Lerp(m_initialZoom, m_targetZoom, m_zoomLerpTimer);
}

// Called to bind functionality to input
void AMetaBallsController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// setup keybinds to control our metaballs
	PlayerInputComponent->BindAction("increaseParam", IE_Pressed, this, &AMetaBallsController::BeginIncreaseParam);
	PlayerInputComponent->BindAction("decreaseParam", IE_Pressed, this, &AMetaBallsController::BeginDecreaseParam);
	PlayerInputComponent->BindAction("increaseParam", IE_Released, this, &AMetaBallsController::EndIncreaseParam);
	PlayerInputComponent->BindAction("decreaseParam", IE_Released, this, &AMetaBallsController::EndDecreaseParam);

	PlayerInputComponent->BindAction("adjustCenterRadius", IE_Pressed, this, &AMetaBallsController::BeginAdjustCenterRadius);
	PlayerInputComponent->BindAction("adjustOrbitRadius", IE_Pressed, this, &AMetaBallsController::BeginAdjustOrbitRadius);
	PlayerInputComponent->BindAction("adjustDistanceFromCenter", IE_Pressed, this, &AMetaBallsController::BeginAdjustDistanceFromCenter);
	PlayerInputComponent->BindAction("adjustNumSpheres", IE_Pressed, this, &AMetaBallsController::BeginAdjustNumSpheres);
	PlayerInputComponent->BindAction("adjustOrbitsPerSecond", IE_Pressed, this, &AMetaBallsController::BeginAdjustOrbitsPerSecond);
	PlayerInputComponent->BindAction("adjustOrbitSpeedOffset", IE_Pressed, this, &AMetaBallsController::BeginAdjustOrbitSpeedOffset);
	PlayerInputComponent->BindAction("adjustOrbitBeginOffset", IE_Pressed, this, &AMetaBallsController::BeginAdjustOrbitBeginOffset);
	PlayerInputComponent->BindAction("adjustRipple", IE_Pressed, this, &AMetaBallsController::BeginAdjustRipple);

	PlayerInputComponent->BindAction("adjustCenterRadius", IE_Released, this, &AMetaBallsController::EndAdjustCenterRadius);
	PlayerInputComponent->BindAction("adjustOrbitRadius", IE_Released, this, &AMetaBallsController::EndAdjustOrbitRadius);
	PlayerInputComponent->BindAction("adjustDistanceFromCenter", IE_Released, this, &AMetaBallsController::EndAdjustDistanceFromCenter);
	PlayerInputComponent->BindAction("adjustNumSpheres", IE_Released, this, &AMetaBallsController::EndAdjustNumSpheres);
	PlayerInputComponent->BindAction("adjustOrbitsPerSecond", IE_Released, this, &AMetaBallsController::EndAdjustOrbitsPerSecond);
	PlayerInputComponent->BindAction("adjustOrbitSpeedOffset", IE_Released, this, &AMetaBallsController::EndAdjustOrbitSpeedOffset);
	PlayerInputComponent->BindAction("adjustOrbitBeginOffset", IE_Released, this, &AMetaBallsController::EndAdjustOrbitBeginOffset);
	PlayerInputComponent->BindAction("adjustRipple", IE_Released, this, &AMetaBallsController::EndAdjustRipple);
}

void AMetaBallsController::BeginIncreaseParam()
{
	m_increasingParams = true;
	m_decreasingParams = false;
	IncrementParam(1.0f); // intentionally trigger increment immediately so that player spamming the button is responsive
}

void AMetaBallsController::BeginDecreaseParam()
{
	m_increasingParams = false;
	m_decreasingParams = true;
	IncrementParam(-1.0f); // intentionally trigger increment immediately so that player spamming the button is responsive
}

void AMetaBallsController::EndIncreaseParam()
{
	m_increasingParams = false;
}

void AMetaBallsController::EndDecreaseParam()
{
	m_decreasingParams = false;
}

void AMetaBallsController::BeginAdjustCenterRadius()
{
	m_state = EAdjustmentState::CENTER_RADIUS;
}

void AMetaBallsController::BeginAdjustOrbitRadius()
{
	m_state = EAdjustmentState::ORBIT_RADIUS;
}

void AMetaBallsController::BeginAdjustDistanceFromCenter()
{
	m_state = EAdjustmentState::DISTANCE_FROM_CENTER;
}

void AMetaBallsController::BeginAdjustNumSpheres()
{
	m_state = EAdjustmentState::NUM_SPHERES;
}

void AMetaBallsController::BeginAdjustOrbitsPerSecond()
{
	m_state = EAdjustmentState::ORBITS_PER_SECOND;
}

void AMetaBallsController::BeginAdjustOrbitSpeedOffset()
{
	m_state = EAdjustmentState::ORBIT_SPEED_OFFSET;
}

void AMetaBallsController::BeginAdjustOrbitBeginOffset()
{
	m_state = EAdjustmentState::ORBIT_BEGIN_OFFSET;
}

void AMetaBallsController::BeginAdjustRipple()
{
	m_state = EAdjustmentState::RIPPLE;
}

void AMetaBallsController::EndAdjustCenterRadius()
{
	if(m_state == EAdjustmentState::CENTER_RADIUS)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustOrbitRadius()
{
	if (m_state == EAdjustmentState::ORBIT_RADIUS)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustDistanceFromCenter()
{
	if (m_state == EAdjustmentState::DISTANCE_FROM_CENTER)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustNumSpheres()
{
	if (m_state == EAdjustmentState::NUM_SPHERES)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustOrbitsPerSecond()
{
	if (m_state == EAdjustmentState::ORBITS_PER_SECOND)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustOrbitSpeedOffset()
{
	if (m_state == EAdjustmentState::ORBIT_SPEED_OFFSET)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustOrbitBeginOffset()
{
	if (m_state == EAdjustmentState::ORBIT_BEGIN_OFFSET)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::EndAdjustRipple()
{
	if (m_state == EAdjustmentState::RIPPLE)
	{
		m_state = EAdjustmentState::ZOOM;
	}
}

void AMetaBallsController::IncrementParam(float multiplier)
{
	m_incrementTimer = 0.0f;

	switch (m_state)
	{
	case EAdjustmentState::CENTER_RADIUS:
		m_targetCenterRadius = FMath::Max(m_centerRadiusIncrementAmount, m_targetCenterRadius + (m_centerRadiusIncrementAmount * multiplier));
		m_initialCenterRadius = m_centerRadius;
		m_centerRadiusLerpTimer = 0.0f;
		break;
	case EAdjustmentState::DISTANCE_FROM_CENTER:
		m_targetDistanceFromCenter = FMath::Max(0.0f, m_targetDistanceFromCenter + (m_distanceFromCenterIncrementAmount * multiplier));
		m_initialDistanceFromCenter = m_distanceFromCenter;
		m_distanceFromCenterLerpTimer = 0.0f;
		break;
	case EAdjustmentState::NUM_SPHERES:
		m_targetNumSpheres = FMath::Max(m_numSpheresIncrementAmount, m_targetNumSpheres + (m_numSpheresIncrementAmount * multiplier));
		m_initialNumSpheres = m_numSpheres;
		break;
	case EAdjustmentState::ORBITS_PER_SECOND:
		m_targetOrbitsPerSecond = FMath::Max(0.0f, m_targetOrbitsPerSecond + (m_orbitsPerSecondIncrementAmount * multiplier));
		m_initialOrbitsPerSecond = m_orbitsPerSecond;
		m_orbitsPerSecondLerpTimer = 0.0f;
		break;
	case EAdjustmentState::ORBIT_BEGIN_OFFSET:
		m_targetOrbitBeginOffset = FMath::Max(0.0f, m_targetOrbitBeginOffset + (m_orbitBeginOffsetIncrementAmount * multiplier));
		m_initialOrbitBeginOffset = m_orbitBeginOffset;
		m_orbitBeginOffsetLerpTimer = 0.0f;
		break;
	case EAdjustmentState::ORBIT_RADIUS:
		m_targetOrbitRadius = FMath::Max(m_orbitRadiusIncrementAmount, m_targetOrbitRadius + (m_orbitRadiusIncrementAmount * multiplier));
		m_initialOrbitRadius = m_orbitRadius;
		m_orbitRadiusLerpTimer = 0.0f;
		break;
	case EAdjustmentState::ORBIT_SPEED_OFFSET:
		m_targetOrbitSpeedOffset = FMath::Max(0.0f, m_targetOrbitSpeedOffset + (m_orbitSpeedOffsetIncrementAmount * multiplier));
		m_initialOrbitSpeedOffset = m_orbitSpeedOffset;
		m_orbitSpeedOffsetLerpTimer = 0.0f;
		break;
	case EAdjustmentState::RIPPLE:
		m_targetRipple = FMath::Max(0.0f, m_targetRipple + (m_rippleIncrementAmount * multiplier));
		m_initialRipple = m_ripple;
		m_rippleLerpTimer = 0.0f;
		break;
	case EAdjustmentState::ZOOM:
		m_targetZoom = FMath::Clamp(m_targetZoom + (m_zoomIncrementAmount * multiplier), m_zoomMin, m_zoomMax);
		m_initialZoom = m_zoom;
		m_zoomLerpTimer = 0.0f;
		break;
	}
}


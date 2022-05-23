// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "BeaconActor.generated.h"

UCLASS()
class DEEPWISIM_API ABeaconActor : public AActor
{
	GENERATED_BODY()
	/**  */
	
public:	
	// Sets default values for this actor's properties
	ABeaconActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

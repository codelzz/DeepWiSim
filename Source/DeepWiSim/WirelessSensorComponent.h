// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Math2DFunctionLibrary.h"
#include "RayTracingFunctionLibrary.h"
#include "BeaconActor.h"
#include "UdpServerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "WirelessSensorComponent.generated.h"


USTRUCT()
struct FJsonWirelessSensorMeasurement
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		FString Source;

	UPROPERTY()
		FString Owner;

	UPROPERTY()
		TMap<FString, float> Measurement;

	UPROPERTY()
		TMap<FString, float> Coordinates;

	UPROPERTY()
		FString Timestamp;

	FJsonWirelessSensorMeasurement()
	{
		Measurement.Empty();
		Coordinates.Empty();
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEEPWISIM_API UWirelessSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWirelessSensorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** Activate sensor and start measuring signal. */
	UFUNCTION(BlueprintCallable)
	void ActivateSensor();

	/** Deactivate sensor. */
	UFUNCTION(BlueprintCallable)
	void DeactivateSensor();

	/** The main function for measuring sensor which was run on thread. */
	UFUNCTION(BlueprintCallable)
	void Measure();

	/** Get the measurement json string. */
	UFUNCTION(BlueprintCallable)
	FString GetMeasurementJsonString() { return MeasurementString; }

	/** Get the DPI. */
	UFUNCTION(BlueprintCallable)
	int32 GetDPI() { return DPI; }

	/** Set the DPI. */
	UFUNCTION(BlueprintCallable)
	void SetDPI(int32 InDPI) { DPI = InDPI; }

	/** Get the trace distance. */
	UFUNCTION(BlueprintCallable)
	float GetTraceDistance() { return MaxRayTracingDistance; }

	/** Set the trace distance. */
	UFUNCTION(BlueprintCallable)
	void SetTraceDistance(float InTraceDistance) { MaxRayTracingDistance = InTraceDistance; }

	/** Get the trace depth */
	UFUNCTION(BlueprintCallable)
	int32 GetTraceDepth() { return MaxRayTracingDepth; }

	/** Set the trace depth */
	UFUNCTION(BlueprintCallable) 
	void SetTraceDepth(int32 InTraceDepth) { MaxRayTracingDepth = InTraceDepth; }

	/** Get the frequency */
	UFUNCTION(BlueprintCallable)
	float GetFrequency() { return Frequency; }

	/** Set the frequency */
	UFUNCTION(BlueprintCallable) 
	void SetFrequency(float InFrequency) { Frequency = InFrequency; }

	/** Get the frequency */
	UFUNCTION(BlueprintCallable)
	int32 GetTransmitPower() { return TransmitPower; }

	/** Set the frequency */
	UFUNCTION(BlueprintCallable)
	void SetTransmitPower(int32 InTransmitPower) { TransmitPower = InTransmitPower; }

private:

	/** Set the server handle for sending measurement to client*/
	void FindServerHandle()
	{
		ServerHandle = GetOwner()->FindComponentByClass<UUdpServerComponent>();
		if (ServerHandle == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] missing UdpServerComponent"), *GetOwner()->GetName())
		}
	}

	/** Find All Signal Sources */
	void FindAllSignalSources(){ SignalSources = GetAllSources(); }

	/** 
	* Get All Signal Sources which can be detected. 
	* 
	* @return Signal Source Array
	*/
	TArray<AActor*> GetAllSources() 
	{
		TArray<AActor*> Sources;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABeaconActor::StaticClass(), Sources);
		return Sources;
	}

	/** 
	* Get The number of channel for each sensor, 
	* the channel size equivenent to the totoal number of source
	* This must be called after FindAllSignalSources()
	* 
	* @return ChannelSize
	*/
	void FindChannelSize() {ChannelSize = SignalSources.Num();}

	/**
	* Packet raw measurement with corresponding source id (beacon name)
	* 
	* @param Measurement the raw measurement without source information
	* @return Measurement with source id
	*/
	TMap<FString, float> PacketMeasurement(const FFloatArray1D& Measurement) const;

	/**
	* Packet component location as ground to position of sensor
	*/
	TMap<FString, float> PacketCoordinates() const;

	/**
	* Send Meausrement to udp client if possible
	* 
	* @return True if send success, false otherwise
	*/
	bool SendMeasurement(const FFloatArray1D& Measurement);

public:
	//~ Begin Ray Trace
	void DrawDebugRay(const FRay_& Ray, float Distance, FColor Color, float Thickness = 0.5f);

	/** Base on given DPI generate Trace directions */
	void SetTraceDirections();
	TArray<FVector> GeneratePointsByFibonacciSphereAlgorithm(int32 N);

	/** 
	* Ray Tracing Algorithm.
	* 
	* @param Origin The initial start point of rays
	* @param Directions All direction for tracing
	*/
	FFloatArray1D RayTracing(const FVector& Origin, const TArray<FVector>& Directions);
	FFloatArray1D DirectRayTracing(const FVector& Origin);
	FFloatArray2D IndirectRayTracing(const FVector& Origin, const TArray<FVector>& Directions);
	FFloatArray1D RayTracingBySingleRay(const FRay_& Ray, float Distance, int32 Depth=0);

	/**
	* Calculate the Intensity at given fragment location.
	* 
	* @param FragRay The Ray start from frag and use fragment normal as direction 
	* @param ViewLocation The location where ray(from view to fragment) start from
	* @param Distance The max distance the ray(from fragment to radiation source) can reach
	* @return Intensity
	*/
	FFloatArray1D GetIntensityAt(const FRay_ FragRay, const FVector& ViewLocation, float Distance);

	/** 
	* Wapper for built-in function LineTraceSingleByChannel, Collision Channels fix to PhysicsBody
	* 
	* @param Ray The Ray for tracing
	* @param Distance The maximum tracing range
	* @param ActorsTOIngore A list of actors need to be ignore while tracing
	* @return HitResult A struct holds hit point information
	*/
	FHitResult RayTraceSingleByChannel(const FRay_& Ray, float Distance, const AActor* ActorsToIgnore = NULL);
	//~ End Ray Trace

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DPI = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta=(DisplayName="Ray-tracing Distance (cm)"))
		float MaxRayTracingDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta=(DisplayName="Ray-tracing Depth"))
		int32 MaxRayTracingDepth = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Interval = 0.025f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Frequency (GHz)"))
		float Frequency = 2.45;  /* 2.45 GHz for Bluetooth by Default */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (DisplayName = "Transmit Power (dBm)"))
		int32 TransmitPower = -27;  /* -27 dBm for Bluetooth by Default */


private:
	/** Timer Delegate bind with sensing function. */
	FTimerHandle TimerHandle;
	UUdpServerComponent* ServerHandle;

protected:
	int32 ChannelSize;
	TArray<AActor*> SignalSources;
	TArray<FVector> TraceDirections;
	FString MeasurementString = TEXT("N/A");
	const float RayOffset = 0.01f;

};

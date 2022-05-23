// Fill out your copyright notice in the Description page of Project Settings.


#include "WirelessSensorComponent.h"
#include "WirelessSignalFunctionLibrary.h"
#include "Math2DFunctionLibrary.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UWirelessSensorComponent::UWirelessSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UWirelessSensorComponent::BeginPlay()
{
	Super::BeginPlay();
	FindServerHandle();
}

// Called every frame
void UWirelessSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWirelessSensorComponent::ActivateSensor()
{
	// Save signal source references for later use 
	FindAllSignalSources();
	// Save the channel size
	FindChannelSize();
	// Generate ray trace directions (evenly distribute on sphere)
	SetTraceDirections();
	// Check if parameter are valid before start thread
	if (SignalSources.Num() <= 0 || DPI <= 0) { return; }
	// Run measure thread
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UWirelessSensorComponent::Measure, Interval, true);
}

void UWirelessSensorComponent::DeactivateSensor()
{
	// Stop measure thread
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}


void UWirelessSensorComponent::DrawDebugRay(const FRay_ &Ray, float Distance, FColor Color, float Thickness)
{
	DrawDebugLine(GetWorld(), Ray.Origin, Ray.PointAt(Distance), Color, false, 0.1f, 0, Thickness);
}

void UWirelessSensorComponent::Measure()
{
	// Run ray tracing algorithm to get the distance measurement result.
	FFloatArray1D Measurement = RayTracing(this->GetComponentLocation(), TraceDirections);
	// Convert Measurement to Received Signal Strength.
	Measurement = UWirelessSignalFunctionLibrary::DistanceArray1DToReceivedSignalStrength(Measurement, TransmitPower, Frequency);
	// Send received signal strength measurement result to connected client.
	SendMeasurement(Measurement);
}

TMap<FString, float> UWirelessSensorComponent::PacketMeasurement(const FFloatArray1D& Measurement) const
{
	// The raw measurement result does not have signal source id
	// here we need to bind measurement to corresponding source.
	check(SignalSources.Num() == Measurement.Num());
	TMap<FString, float> MeasurementMap;
	for (int32 Index=0; Index< SignalSources.Num(); Index++)
	{
		MeasurementMap.Add(SignalSources[Index]->GetName(), Measurement[Index]);
	}
	return MeasurementMap;
}

TMap<FString, float> UWirelessSensorComponent::PacketCoordinates() const
{
	// Get the component location as ground position for client side supervised learning.
	TMap<FString, float> Coordinates;
	FVector Location = GetComponentLocation();
	Coordinates.Add(TEXT("x"), FMath::RoundToInt(Location.X));
	Coordinates.Add(TEXT("y"), FMath::RoundToInt(Location.Y));
	Coordinates.Add(TEXT("z"), FMath::RoundToInt(Location.Z));
	return Coordinates;
}

bool UWirelessSensorComponent::SendMeasurement(const FFloatArray1D& Measurement)
{
	FJsonWirelessSensorMeasurement JsonMeasurement = FJsonWirelessSensorMeasurement();
	JsonMeasurement.Source = GetName();
	JsonMeasurement.Owner = GetOwner()->GetName();
	JsonMeasurement.Coordinates = PacketCoordinates();
	JsonMeasurement.Measurement = PacketMeasurement(Measurement);
	JsonMeasurement.Timestamp = FDateTime::UtcNow().ToString();

	FString JsonString;
	if (FJsonObjectConverter::UStructToJsonObjectString(JsonMeasurement, JsonString, 0, 0))
	{
		MeasurementString = JsonString;
		if (ServerHandle != nullptr && ServerHandle->IsServerValid())
		{
			ServerHandle->Send(JsonString);
			return true;
		}
	}
	return false;
}

// ~ begin Ray Tracing Algorithm
FFloatArray1D UWirelessSensorComponent::RayTracing(const FVector& Origin, const TArray<FVector>& Directions)
{
	// Get the Direct Ray Tracing Result (compute by direct radiation).
	FFloatArray1D DirectRayTracingResult = DirectRayTracing(Origin);
	// Get the In Direct Ray Tracing Result (compute by reflect, diffuse, and ambient radiation).
	FFloatArray2D IndirectRayTracingResult = IndirectRayTracing(Origin, Directions);
	// Merge Direct & Indirect Ray Tracing Result.
	FFloatArray2D Intensity2D = IndirectRayTracingResult;
	Intensity2D.Array.Add(DirectRayTracingResult);
	FFloatArray1D Intensity = Intensity2D.ReduceMax(1);
	// The ray tracing result return the Signal Intensity (Attenuation) on given direction (ray)
	// We need to convert it back to distance as (noisy) measurement result.
	FFloatArray1D Distance = UWirelessSignalFunctionLibrary::AttenuationArray1DToDistance(Intensity, 2.0);
	return Distance;
}

FFloatArray1D UWirelessSensorComponent::DirectRayTracing(const FVector& Origin)
{
	FFloatArray1D Intensity;
	Intensity.Init(ChannelSize);
	FVector SensorLocation = this->GetComponentLocation();
	// Calcualte the direct radiation intensity.
	for (int32 Index=0; Index<SignalSources.Num(); Index++)
	{
		FVector SourceLocation = SignalSources[Index]->GetActorLocation();
		float SensorToSourceDistance = FVector::Distance(SourceLocation, SensorLocation);
		// [Skip] if distance between sensor and source large than the max ray trace range (the distance is out of reachale range).
		if (SensorToSourceDistance > MaxRayTracingDistance) { continue; }
		// Create a Ray from sensor pointing to source
		FRay_ Ray = FRay_(SensorLocation, SourceLocation - SensorLocation);
		// Get the hit result
		FHitResult Hit = RayTraceSingleByChannel(Ray, SensorToSourceDistance, NULL);
		// [Skip] if the ray was blocked by anything.
		if (Hit.bBlockingHit) {continue;}
		// Otherwise calculate the Intensity
		Intensity[Index] = UWirelessSignalFunctionLibrary::GetDirectRadiationIntensity(1.0f, FVector::Distance(Hit.TraceStart, Hit.TraceEnd));
		// Create debug line only display if reachable
		DrawDebugRay(Ray, SensorToSourceDistance, FColor(255, 255, 0), 2);
	}
	return Intensity;
}

FFloatArray2D UWirelessSensorComponent::IndirectRayTracing(const FVector& Origin, const TArray<FVector>& Directions)
{
	FFloatArray2D Intensity2D;
	Intensity2D.Init(Directions.Num(), ChannelSize);
	FVector SensorLocation = this->GetComponentLocation();
	// loop over all directions, for each direction run ray tracing to calculate Intensity
	for (int32 Index=0; Index < Directions.Num(); Index++)
	{
		FRay_ Ray = FRay_(SensorLocation, Directions[Index], true);
		// Run ray tracing on single direction
		Intensity2D[Index] = RayTracingBySingleRay(Ray, MaxRayTracingDistance);
	}
	return Intensity2D;
}

FFloatArray1D UWirelessSensorComponent::RayTracingBySingleRay(const FRay_& Ray, float Distance, int32 Depth)
{
	FFloatArray1D Intensity;
	Intensity.Init(ChannelSize);
	FHitResult Hit = RayTraceSingleByChannel(Ray, Distance, NULL);
	// [skip] if not hit anything. (there is not reflection/diffuse/ambient happen on this direction)
	if (!Hit.bBlockingHit) {return Intensity;}
	// Get fragment(the piece being hit) information
	float FragDistance = Hit.Distance;
	FRay_ FragRay = FRay_(Hit.Location, Hit.Normal, true);
	// Compute the remaining distance for deeper tracing
	float RemainingDistance = FMath::Max(0.0f, Distance - Hit.Distance);
	// Calculate intensity at fragment location
	Intensity = GetIntensityAt(FragRay, Ray.Origin,  RemainingDistance);
	// Check if recursive function not yet reach the max depath and has remaining distance
	if (Depth < MaxRayTracingDepth && RemainingDistance > 0)
	{
		// Run ray tracing again on reflected direction until reach the max trace depth
		FRay_ ReflectionRay = FRay_(FragRay.Origin, FMath::GetReflectionVector(Ray.Direction, FragRay.Direction));
		/** [IMPORTANT] Small offset apply on reflection direction to prevent hit at start point (the ray cannot get out if no offset)*/
		ReflectionRay.Origin = ReflectionRay.PointAt(RayOffset);
		Intensity += RayTracingBySingleRay(ReflectionRay, RemainingDistance, Depth + 1);
	}
	//~ begin debug code
	if (Intensity.Max() > 0) { 
		float Thickness = 2/(1 - FMath::LogX(10, Intensity.Max()));
		DrawDebugRay(FragRay, 5, FColor(0, 0, 255), Thickness);
		DrawDebugRay(Ray, FragDistance, FColor(255, 0, 0), Thickness); 
	}
	//~ end debug code
	return Intensity;
}

FFloatArray1D UWirelessSensorComponent::GetIntensityAt(const FRay_ FragRay, const FVector& ViewLocation, float Distance)
{
	FFloatArray1D Intensity;
	Intensity.Init(ChannelSize);
	FRay_ ToViewRay = FRay_(FragRay.Origin, FragRay.Origin - ViewLocation);
	// Loop over all radiation sources
	for (int32 Index = 0; Index < SignalSources.Num(); Index++)
	{
		// Compute distance between source and fragment
		FVector SourceLocation = SignalSources[Index]->GetActorLocation();
		float SourceFragDistance = FVector::Distance(SourceLocation, FragRay.Origin);
		// [skip] if distance between source and frag larger that remaining trace distance (not reachable)
		if (SourceFragDistance > Distance) { continue; }
		// Make a ray from fragment point to source
		FRay_ ToSourceRay = FRay_(FragRay.Origin, SourceLocation - FragRay.Origin);
		/**
		* [IMPORTANT] Small offset apply on Trace direction to prevent hit at start point 
		*/
		ToSourceRay.Origin = ToSourceRay.PointAt(RayOffset);
		FHitResult Hit = RayTraceSingleByChannel(ToSourceRay, Distance, NULL);
		// [skip] if hit any thing between source and frag
		if (Hit.bBlockingHit && Hit.Distance < SourceFragDistance) { continue; }
		// if Ray can reach the source
		Intensity[Index] = UWirelessSignalFunctionLibrary::CalculateIntensity(ToSourceRay, ToViewRay, FragRay.Direction, SourceLocation, ViewLocation);
		//~ start debug Ray
		float Thickness = 2/(1 - FMath::LogX(10, Intensity[Index]));
		DrawDebugRay(ToSourceRay, SourceFragDistance , FColor(0, 255, 0), Thickness);
		//~ end debug Ray
	}
	return Intensity;
}

FHitResult UWirelessSensorComponent::RayTraceSingleByChannel(const FRay_& Ray, float Distance, const AActor* ActorsToIgnore)
{
	// A wapper for built-in collision detection function
	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), false, ActorsToIgnore);
	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(
		Hit,
		Ray.Origin,
		Ray.PointAt(Distance),
		ECollisionChannel::ECC_Visibility,
		TraceParams,
		ECollisionResponse::ECR_Block
	);
	return Hit;
}
//~ end Ray Tracing


void UWirelessSensorComponent::SetTraceDirections()
{
	check(DPI > 1);
	TraceDirections = GeneratePointsByFibonacciSphereAlgorithm(DPI);
}

TArray<FVector> UWirelessSensorComponent::GeneratePointsByFibonacciSphereAlgorithm(int32 N)
{
	// Reference: https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere
	TArray<FVector> Points;
	float Phi = PI * (3.0f - FMath::Sqrt(0.5f));
	for (int32 Index = 0; Index < N; Index++)
	{
		float Y = 1 - (float(Index) / float(N - 1)) * 2;
		float Radius = FMath::Sqrt(1.0f - Y * Y);
		float Theta = Phi * float(Index);
		float X = FMath::Cos(Theta) * Radius;
		float Z = FMath::Sin(Theta) * Radius;
		Points.Add(FVector(X, Y, Z).GetSafeNormal());
	}
	return Points;
}


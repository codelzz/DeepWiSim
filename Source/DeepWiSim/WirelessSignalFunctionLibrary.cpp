// Fill out your copyright notice in the Description page of Project Settings.


#include "WirelessSignalFunctionLibrary.h"

FFloatArray1D UWirelessSignalFunctionLibrary::AttenuationArray1DToDistance(const FFloatArray1D& AttenuationArray, float PathLossExponent)
{
	FFloatArray1D DistanceArray;
	DistanceArray.InitLike(AttenuationArray);
	for (int32 Index = 0; Index < AttenuationArray.Num(); Index++)
	{
		DistanceArray[Index] = AttenuationToDistance(AttenuationArray[Index], PathLossExponent);
	}
	return DistanceArray;
}

FFloatArray2D UWirelessSignalFunctionLibrary::AttenuationArray2DToDistance(const FFloatArray2D& AttenuationArray, float PathLossExponent)
{
	FFloatArray2D DistanceArray;
	DistanceArray.InitLike(AttenuationArray);
	for (int32 Index = 0; Index < AttenuationArray.Num0(); Index++)
	{
		DistanceArray[Index] = AttenuationArray1DToDistance(AttenuationArray[Index], PathLossExponent);
	}
	return DistanceArray;

}

float UWirelessSignalFunctionLibrary::AttenuationToDistance(float Attenuation, float PathLossExponent)
{
	if (Attenuation <= 0) { return -1; }
	if (PathLossExponent == 0) { PathLossExponent = 0.0001; }
	return FMath::Pow(1/Attenuation, 1/PathLossExponent) * 100;
}

/*
float UWirelessSignalFunctionLibrary::AttenuationToReceivedSignalStrength(float Attenuation, float ReferencePowerAt1Meter)
{
	return FMath::LogX(10.0f, Attenuation) * 10.0f + ReferencePowerAt1Meter;
}
*/

float UWirelessSignalFunctionLibrary::AttenuationToReceivedSignalStrength(float Attenuation, float TransmitPower, float FrequencyInGHz, float PathLossExponent)
{
	float FrequencyInHz = FrequencyInGHz * 1e9;
	return FMath::LogX(10.0f, Attenuation) * 10.0f + TransmitPower - 10 * PathLossExponent * FMath::LogX(10.0f, 4.0f * PI * FrequencyInHz / C);
}


float UWirelessSignalFunctionLibrary::CalculateIntensity(const FRay_& ToSourceRay, const FRay_& ToViewRay, const FVector& FragNormal,
	const FVector& SourceLocation, const FVector& ViewLocation)
{
	// We assume almost no ambient radiation
	float Ambient = GetAmbientRadiationIntensity(0, 0);
	//Diffuse
	float Diffuse = GetDiffuseRadiationIntensity(1, ToSourceRay.Direction, FragNormal, 0.5);
	float Specular = GetSpecularRadiationIntensity(1, ToSourceRay.Direction, ToViewRay.Direction, FragNormal, 0.5);
	float Distance = FVector::Distance(ToSourceRay.Origin, SourceLocation) + FVector::Distance(ToViewRay.Origin, ViewLocation);
	return (Ambient + Diffuse + Specular) * DistanceToAttenuation(Distance, 2.0f);
}

/*
FFloatArray1D UWirelessSignalFunctionLibrary::DistanceArray1DToReceivedSignalStrength(const FFloatArray1D& DistanceArray, float PathLossExponent, float ReferencePowerAt1Meter, bool bRound)
{
	FFloatArray1D ReceivedSignalStrength;
	ReceivedSignalStrength.Init(DistanceArray.Num());
	for (int32 Index = 0; Index < DistanceArray.Num(); Index++)
	{
		ReceivedSignalStrength.Array[Index] = DistanceToReceivedSignalStrength(DistanceArray.Array[Index],
			PathLossExponent, ReferencePowerAt1Meter, bRound);
	}
	return ReceivedSignalStrength;
}
*/

FFloatArray1D UWirelessSignalFunctionLibrary::DistanceArray1DToReceivedSignalStrength(const FFloatArray1D& DistanceArray, float TransmitPower, float FrequencyInGHz, float PathLossExponent, bool bRound)
{
	FFloatArray1D ReceivedSignalStrength;
	ReceivedSignalStrength.Init(DistanceArray.Num());
	for (int32 Index = 0; Index < DistanceArray.Num(); Index++)
	{
		ReceivedSignalStrength.Array[Index] = DistanceToReceivedSignalStrength(DistanceArray.Array[Index],
			TransmitPower, FrequencyInGHz, PathLossExponent, bRound);
	}
	return ReceivedSignalStrength;
}


float UWirelessSignalFunctionLibrary::DistanceToAttenuation(float Distance, float PathLossExponent)
{
	// Distance less that 1 unreal unit (centimeter) return 1 as no attenuation effect 
	if (Distance < 1) {return 1;}
	// convert to meter scale before compute result
	return FMath::Pow(Distance * 0.01f, -1.0f * PathLossExponent);
}

/*
float UWirelessSignalFunctionLibrary::DistanceToReceivedSignalStrength(float Distance, float PathLossExponent, float ReferencePowerAt1Meter, bool bRound)
{
	// return min measurable power if no measurement
	if (Distance <= 0) { return -100.0f; } 	
	// convert distance to meter before convert to RSS
	float ReceivedSignalStrength = ReferencePowerAt1Meter - 10 * PathLossExponent * FMath::LogX(10.0f, Distance * 0.01 );
	// clamp value less than -100.0f
	ReceivedSignalStrength = FMath::Max(ReceivedSignalStrength, -100.0f);
	if (bRound)
		ReceivedSignalStrength = float(FMath::RoundToInt(ReceivedSignalStrength));
	return ReceivedSignalStrength;
}
*/

float UWirelessSignalFunctionLibrary::DistanceToReceivedSignalStrength(float Distance, float TransmitPower, float FrequencyInGHz, float PathLossExponent, bool bRound)
{
	// return min measurable power if no measurement
	if (Distance <= 0) { return -100.0f; } 	
	float DistanceInMeter = Distance * 0.01;
	float FrequencyInHz = FrequencyInGHz * 1e9;
	// convert distance to meter before convert to RSS
	float ReceivedSignalStrength = TransmitPower - 10 * PathLossExponent * FMath::LogX(10.0f,4.0f * PI * DistanceInMeter * FrequencyInHz / C);
	// clamp value less than -100.0f
	ReceivedSignalStrength = FMath::Max(ReceivedSignalStrength, -100.0f);
	if (bRound)
		ReceivedSignalStrength = float(FMath::RoundToInt(ReceivedSignalStrength));
	return ReceivedSignalStrength;

}


float UWirelessSignalFunctionLibrary::GetAmbientRadiationIntensity(float Intensity, float Coefficient)
{
	return Intensity * Coefficient;
}

float UWirelessSignalFunctionLibrary::GetDiffuseRadiationIntensity(float Intensity, FVector SourceDirection, FVector FragNormal, float Coefficient)
{
	return Intensity * Coefficient *  FMath::Max(FVector::DotProduct(SourceDirection, FragNormal), 0.0f);
}

float UWirelessSignalFunctionLibrary::GetDirectRadiationIntensity(float Intensity, float Distance)
{
	return Intensity * DistanceToAttenuation(Distance, 2.0f);
}

float UWirelessSignalFunctionLibrary::GetSpecularRadiationIntensity(float Intensity, FVector SourceDirection, FVector ViewDirection, FVector FragNormal, 
	float Coefficient, float Shininess)
{
	FVector HalfwayDirection = (SourceDirection + ViewDirection).GetSafeNormal();
	return Intensity * Coefficient * FMath::Pow(FMath::Max(0.0f, FVector::DotProduct(FragNormal, HalfwayDirection)), Shininess);
}



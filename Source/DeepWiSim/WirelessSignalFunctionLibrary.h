// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math2DFunctionLibrary.h"
#include "RayTracingFunctionLibrary.h"
#include "WirelessSignalFunctionLibrary.generated.h"



/**
 * 
 */
UCLASS(Blueprintable)
class DEEPWISIM_API UWirelessSignalFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
	/* The Speed of Light */
	static const int C = 299792458;

public:
	/** Attenuation Array To Distance */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static FFloatArray1D AttenuationArray1DToDistance(const FFloatArray1D& AttenuationArray, float PathLossExponent);

	/** Attenuation Array2D To Distance */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static FFloatArray2D AttenuationArray2DToDistance(const FFloatArray2D& AttenuationArray, float PathLossExponent);

	/** Calculate distance based on attenuation */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static float AttenuationToDistance(float Attenuation = 1.0f, float PathLossExponent = 2.0f);

	/** Calculate Received Signal Strength */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		//static float AttenuationToReceivedSignalStrength(float Attenuation = 1.0f, float ReferencePowerAt1Meter = -67.0f);
		static float AttenuationToReceivedSignalStrength(float Attenuation, float TransmitPower, float FrequencyInGHz, float PathLossExponent = 2.0f);

	/** Calculate Intensity */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
	static float CalculateIntensity(const FRay_& ToSourceRay, const FRay_& ToViewRay, const FVector& FragNormal,
		const FVector& SourceLocation, const FVector& ViewLocation);

	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		//static FFloatArray1D DistanceArray1DToReceivedSignalStrength(const FFloatArray1D& DistanceArray, float PathLossExponent = 2.0f ,float ReferencePowerAt1Meter = -67.0f, bool bRound = true);
		static FFloatArray1D DistanceArray1DToReceivedSignalStrength(const FFloatArray1D& DistanceArray, float TransmitPower, float FrequencyInGHz, float PathLossExponent = 2.0f, bool bRound = true);
	
	/** Calculate attenuation based on distance */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static float DistanceToAttenuation(float Distance = 100.0f, float PathLossExponent = 2.0f);

	/** Calculate attenuation based on distance */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		// static float DistanceToReceivedSignalStrength(float Distance, float PathLossExponent = 2.0f, float ReferencePowerAt1Meter = -67.0f, bool bRound = true);
		static float DistanceToReceivedSignalStrength(float Distance, float TransmitPower, float FrequencyInGHz, float PathLossExponent = 2.0f, bool bRound = true);

	/** Ambient Radiation.  */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static float GetAmbientRadiationIntensity(float Intensity, float Coefficient = 0);

	/** Diffuse Radiation. */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static float GetDiffuseRadiationIntensity(float Intensity, FVector SourceDirection, FVector FragNormal, float Coefficient = 1);

	/** Direct Radiation */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static float GetDirectRadiationIntensity(float Intensity, float Distance);

	/** Specular Radiation */
	UFUNCTION(BlueprintCallable, Category = "Wireless Signal")
		static float GetSpecularRadiationIntensity(float Intensity, FVector SourceDirection, FVector ViewDirection, FVector FragNormal,
			float Coefficient = 0, float Shininess = 16);


};

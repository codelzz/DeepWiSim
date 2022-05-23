// Fill out your copyright notice in the Description page of Project Settings.

// Modify From Ray.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math2DFunctionLibrary.h"
#include "RayTracingFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FRay_ 
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ray Tracing")
		FVector Origin;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ray Tracing")
		FVector Direction;

public:
	FRay_()
	{
		this->Origin = FVector::ZeroVector;
		this->Direction = FVector(0, 0, 1);
	}
	
	/** 
	  * Initialize Ray with origin and direction
	  *
	  * @param Origin Ray Origin Point
	  * @param Direction Ray Direction Vector
	  * @param bDirectionIsNormalized Direction will be normalized unless this is passed as true (default false)
	  */
	FRay_ (const FVector& Origin, const FVector& Direction, bool bDirectionIsNormalized = false)
	{
		this->Origin = Origin;
		this->Direction = Direction;
		if (bDirectionIsNormalized == false)
		{
			this->Direction.Normalize();    // is this a full-accuracy sqrt?
		}
	}

public:

	/** 
	 * Calculate position on ray at given distance/parameter
	 *
	 * @param RayParameter Scalar distance along Ray
	 * @return Point on Ray
	 */
	FVector PointAt(float RayParameter) const
	{
		return Origin + RayParameter * Direction;
	}

	/**
	 * Calculate ray parameter (distance from origin to closest point) for query Point
	 *
	 * @param Point query Point
	 * @return distance along ray from origin to closest point
	 */
	float GetParameter(const FVector& Point) const
	{
		return FVector::DotProduct((Point - Origin), Direction);
	}

	/**
	 * Find minimum squared distance from query point to ray
	 *
	 * @param Point query Point
	 * @return squared distance to Ray
	 */
	float DistSquared(const FVector& Point) const
	{
		float RayParameter = FVector::DotProduct((Point - Origin), Direction);
		if (RayParameter < 0)
		{
			return FVector::DistSquared(Origin, Point);
		}
		else 
		{
			FVector ProjectionPt = Origin + RayParameter * Direction;
			return FVector::DistSquared(ProjectionPt, Point);
		}
	}

	/**
	 * Find closest point on ray to query point
	 * @param Point query point
	 * @return closest point on Ray
	 */
	FVector ClosestPoint(const FVector& Point) const
	{
		float RayParameter = FVector::DotProduct((Point - Origin), Direction);
		if (RayParameter < 0) 
		{
			return Origin;
		}
		else 
		{
			return Origin + RayParameter * Direction;
		}
	}

	/** Return the String representation of a Ray*/
	FString ToString()
	{
		return FString::Printf(TEXT("<Ray Origin=(%.4f, %.4f, %.4f) Direction=(%.4f, %.4f, %.4f)>"),
			Origin.X, Origin.Y, Origin.Z, Direction.X, Direction.Y, Direction.Z);
	}

	FRay_ GetReflectionRay(const FVector &Location, const FVector& SurfaceNormal)
	{
		return FRay_(Location, FMath::GetReflectionVector(Direction, SurfaceNormal));
	}
};


/**
 * 
 */
UCLASS(Blueprintable)
class DEEPWISIM_API URayTracingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/* format a Ray to string */
	UFUNCTION(BlueprintCallable, Category = "Ray Tracing")
	static FString RayToString(FRay_ Ray) { return Ray.ToString();}

	UFUNCTION(BlueprintCallable, Category = "Ray Tracing")
	static FFloatArray1D RayTracing(const FVector Origin, const TArray<FVector>& Directions);

	static FFloatArray1D DirectRayTracing(const FVector Origin);
	static FFloatArray1D IndirectRayTracing(const FVector Origin, const TArray<FVector>& Directions);

	/** Get the number of channels which equivenlent to the number of beacon in the world */
	static int32 GetRayTracingChannelNum();
};

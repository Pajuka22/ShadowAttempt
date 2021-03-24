# ShadowAttempt

Developed with Unreal Engine 4
This project uses a modified Vector.h file. Replace your Engine Vector.h file's contentis with those of Vector_4_26.h. 
Your engine file should be located at Program Files\Epic Games\UE_4.26\Engine\Source\Runtime\Core\Public\Math\Vector.h
Once that's done it should run just fine.

Alternatively, manually add these 2 functions

Declarations:

/**
* Get the angle between this vector and another one.
* @param V The vector to check against.
*/
float RadiansToVector(const FVector& V) const;

/*
* Get the distance of a vector in any given direction.
* @param D Direction
*/
float DistanceInDirection(const FVector& D) const;


Definitions: 
	
FORCEINLINE float FVector::RadiansToVector(const FVector& V) const
{
    return FMath::Acos((X * V.X + Y * V.Y + Z * V.Z) / Size() / V.Size());
}

FORCEINLINE float FVector::DistanceInDirection(const FVector& D) const
{
    return (X * D.X + Y * D.Y + Z * D.Z) / D.Size();
}
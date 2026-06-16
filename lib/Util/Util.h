#pragma once
#include "Type.h"
#include <cmath>


namespace Util
{
	double CalculateRotation(const FVector& oldPosition, const FVector& newPosition);
	float CalculateSquareDistanceXY(const FVector& a, const FVector& b);
}



#include "Util.h"


double Util::CalculateRotation(const FVector& oldPosition, const FVector& newPosition)
{
	double yaw = 0.0;

	double dx = newPosition.X - oldPosition.X;
	double dy = newPosition.Y - oldPosition.Y;
	if (dx != 0 || dy != 0) {
		// 움직임이 있을 경우에만 calculate rotation
		yaw = std::atan2(dy, dx) * 180 / PI; // 라디안에서 도, 언리얼 degree
	}

	return yaw;
}	
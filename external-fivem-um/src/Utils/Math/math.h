#pragma once

#include "../../Utils/Header.h"

extern bool Vec3_Empty(const Vector3& value);
extern Vector3 Vec3_Transform(Vector3* vIn, Matrix* mIn);
extern Vector3 CalcAngle(Vector3 local_cam, Vector3 to_point);
extern float GetSpeed(Vector3 value);
extern float GetDistance(Vector3 value1, Vector3 value2);
extern bool WorldToScreen(Matrix ViewMatrix, Vector3& vWorld, Vector2& vOut);
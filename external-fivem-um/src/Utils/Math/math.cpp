#include "../../Utils/Header.h"
#include "../Config/config.h"

float GetDistance(Vector3 value1, Vector3 value2)
{
    float num = value1.x - value2.x;
    float num2 = value1.y - value2.y;
    float num3 = value1.z - value2.z;

    return sqrt(num * num + num2 * num2 + num3 * num3);
}

float GetSpeed(Vector3 value)
{
    return sqrt(value.x * value.x + value.y * value.y + value.z * value.z) * 3.6f;
}

bool Vec3_Empty(const Vector3& value)
{
    return value == Vector3(0.f, 0.f, 0.f);
}

Vector3 Vec3_Transform(Vector3* vIn, Matrix* mIn)
{
    Vector3 pOut{};
    pOut.x = vIn->x * mIn->_11 + vIn->y * mIn->_21 + vIn->z * mIn->_31 + 1.f * mIn->_41;
    pOut.y = vIn->x * mIn->_12 + vIn->y * mIn->_22 + vIn->z * mIn->_32 + 1.f * mIn->_42;
    pOut.z = vIn->x * mIn->_13 + vIn->y * mIn->_23 + vIn->z * mIn->_33 + 1.f * mIn->_43;

    return pOut;
};

Vector3 CalcAngle(Vector3 local_cam, Vector3 to_point)
{
    Vector3 vOut{};
    float distance = GetDistance(local_cam, to_point);

    vOut.x = (to_point.x - local_cam.x) / distance;
    vOut.y = (to_point.y - local_cam.y) / distance;
    vOut.z = (to_point.z - local_cam.z) / distance;

    return vOut;
}

bool WorldToScreen(Matrix ViewMatrix, Vector3& vWorld, Vector2& vOut)
{
    Matrix v = ViewMatrix.Transpose();
    Vector4 vec = Vector4(v._21, v._22, v._23, v._24), vec_y = Vector4(v._31, v._32, v._33, v._34), vec_z = Vector4(v._41, v._42, v._43, v._44);
    Vector3 vec_out = Vector3((vec.x * vWorld.x) + (vec.y * vWorld.y) + (vec.z * vWorld.z) + vec.w,
        (vec_y.x * vWorld.x) + (vec_y.y * vWorld.y) + (vec_y.z * vWorld.z) + vec_y.w,
        (vec_z.x * vWorld.x) + (vec_z.y * vWorld.y) + (vec_z.z * vWorld.z) + vec_z.w);

    if (vec_out.z <= 0.001f)
        return false;

    vec_out.z = 1.0f / vec_out.z;
    vec_out.x *= vec_out.z;
    vec_out.y *= vec_out.z;

    const float width = (float)GlobalsConfig.GameRect.right;
    const float height = (float)GlobalsConfig.GameRect.bottom;
    float x_temp = width / 2;
    float y_temp = height / 2;

    vOut.x += x_temp + float(0.5f * vec_out.x * width + 0.5f);
    vOut.y = y_temp - float(0.5f * vec_out.y * height + 0.5f);

    return true;
}
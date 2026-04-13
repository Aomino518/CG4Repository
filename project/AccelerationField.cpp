#include "AccelerationField.h"
#include "DebugDraw.h"

void AccelerationField::DrawDebug(const Vector3& origin)
{
#ifdef _DEBUG
    AABB worldAABB = GetWorldAABB(origin);
    DebugDraw::DrawAABB({ 0.0f, 0.0f, 0.0f }, worldAABB, Vector4(0.2f, 1.0f, 0.4f, 1.0f), DebugDrawMode::Wireframe);
#endif
}

AABB AccelerationField::GetWorldAABB(const Vector3& origin) const
{
    AABB result = area_;

    if (space_ == FieldSpace::Local) {
        result.min = result.min + origin + position_;
        result.max = result.max + origin + position_;
    } else {
        result.min = result.min + position_;
        result.max = result.max + position_;
    }

    return result;
}

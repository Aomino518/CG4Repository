#include "AccelerationField.h"

/*void AccelerationField::DrawDebug(const Vector3& origin)
{

}*/

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

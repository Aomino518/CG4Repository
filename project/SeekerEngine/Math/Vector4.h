#pragma once

struct Vector4 {
	float x, y, z, w;

    // スカラー倍
    Vector4 operator*(float scalar) const {
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    // スカラー倍（右辺）
    friend Vector4 operator*(float scalar, const Vector4& v) {
        return v * scalar;
    }

    // 加算
    Vector4 operator+(const Vector4& rhs) const {
        return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }
};
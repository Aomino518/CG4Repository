#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <assert.h>
#include <string>
#include <vector>
#include <cassert>  
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <unordered_map>
#include <algorithm>
#include "Matrix.h"

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	uint32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

// 三つ組のキー
struct TripletKey {
	uint32_t v, vt, vn;
	bool operator == (const TripletKey&) const = default;
};

struct TripletHash {
	size_t operator()(const TripletKey& k) const noexcept {
		size_t h = 0;
		auto mix = [&](uint32_t x) {
			h ^= std::hash<uint32_t>{}(x)+0x9e3779b97f4a7c15ULL + (h << 5) + (h >> 2);
			};

		mix(k.v);
		mix(k.vt);
		mix(k.vn);
		return h;
	}
};

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

// モデル関係の構造体
struct ModelData {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	MaterialData material;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

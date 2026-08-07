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
#include "Quaternion.h"
#include <unordered_map>
#include <algorithm>
#include "MathFunc.h"
#include <map>
#include <optional>
#include <span>
#include <array>

struct EulerTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
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
	float padding0[3]; // 12byte
	Matrix4x4 uvTransform;
	float shininess;
	float environmentColor;
	float padding1[1];
	float alphaReference = 0.0f;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};

struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex = 0;
};

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

// モデル関係の構造体
struct ModelData {
	std::map <std::string, JointWeightData> skinClusterData;
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	MaterialData material;
	Node rootNode;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius; // 光が届く最大距離
	float decay;
	float padding[2];
};

struct SpotLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance; // 光が届く最大距離
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding;
};

struct Particle {
	EulerTransform transform;
	Vector3 rotateVelocity;
	Vector3 velocity;
	Vector4 color;
	Vector4 startColor;
	Vector4 endColor;
	Vector3 startScale;
	Vector3 endScale;
	float lifeTime;
	float currentTime;
	bool isKeepScale;
};

enum class SpawnShape {
	Box,
	Sphere
};

struct ParticleConfig {
	Vector3 velocity = { 0.0f, 0.0f, 0.0f }; // 速度
	Vector3 minVelocity = { -0.1f, -0.1f, -0.1f }; // 速度の最小値
	Vector3 maxVelocity = { 0.1f,  0.1f,  0.1f }; // 速度の最大値
	Vector3 minOffset = { -0.5f, -0.5f, -0.5f }; // オフセットの最小値
	Vector3 maxOffset = { 0.5f,  0.5f,  0.5f }; // オフセットの最大値
	Vector4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色
	Vector4 endColor = { 1.0f, 1.0f, 1.0f, 0.0f }; // 終了色
	Vector4 startColorMin = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色の最小値
	Vector4 startColorMax = { 1.0f, 1.0f, 1.0f, 1.0f }; // 開始色の最大値
	Vector4 endColorMin = { 1.0f, 1.0f, 1.0f, 0.0f };   // 終了色の最小値
	Vector4 endColorMax = { 1.0f, 1.0f, 1.0f, 0.0f };   // 終了色の最大値
	Vector3 startScale = { 1.0f, 1.0f, 1.0f }; // 初期スケール
	Vector3 startScaleMin = { 0.5f, 0.5f, 0.5f }; // 初期スケールの最小値
	Vector3 startScaleMax = { 1.0f, 1.0f, 1.0f }; // 初期スケールの最大値
	Vector3 endScale = { 0.0f, 0.0f, 0.0f }; // 終了スケール
	Vector3 endScaleMin = { 0.5f, 0.5f, 0.5f };   // 終了スケールの最小値
	Vector3 endScaleMax = { 1.5f, 1.5f, 1.5f };   // 終了スケールの最大値
	float lifeTime = 2.0f; // 生存時間
	float minLifeTime = 1.0f; // 生存時間の最小値
	float maxLifeTime = 3.0f; // 生存時間の最大値
	Vector3 rotate = { 0.0f, 0.0f, 0.0f };
	Vector3 minRotate = { 0.0f, 0.0f, 0.0f }; // 回転角の最小値
	Vector3 maxRotate = { 0.0f, 0.0f, 0.0f }; // 回転角の最大値
	Vector3 rotateVelocity = { 0.0f, 0.0f, 0.0f };
	Vector3 minRotateVelocity = { 0.0f, 0.0f, 0.0f }; // 回転速度の最小値
	Vector3 maxRotateVelocity = { 0.0f, 0.0f, 0.0f }; // 回転速度の最大値
	SpawnShape shape = SpawnShape::Box; // 範囲タイプ
	Vector3 boxMin = { -0.5f, -0.5f, -0.5f }; // 箱の最小値
	Vector3 boxMax = { 0.5f,  0.5f,  0.5f }; // 箱の最大値
	float sphereRadius = 1.0f; // 球の半径
	bool isKeepScale = false;
	bool isKeepColor = false;
	bool isKeepVelocity = false;
	bool isKeepRotate = false;
	bool isKeepRotateVelocity = false;
	bool isKeepStartScale = false;
	bool isKeepEndScale = false;
	bool isKeepStartColor = false;
	bool isKeepEndColor = false;
	bool isKeepLifeTime = false;
};

struct ParticleForGPU
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

struct CameraForGPU {
	Vector3 worldPosition;
	float padding;
};

static constexpr uint32_t kMaxPointLights = 512;
static constexpr uint32_t kMaxSpotLights = 512;

struct PointLightGroup {
	PointLight lights[kMaxPointLights];
	int32_t count;
	float pad[3];
};

struct SpotLightGroup {
	SpotLight lights[kMaxSpotLights];
	int32_t count;
	float pad[3];
};

struct DebugVertex {
	Vector4 position;
	Vector4 color;
};

enum class DebugDrawMode {
	Wireframe,
	Solid
};

enum class FieldSpace {
	Local,
	World
};

// キーフレーム
template <typename tValue>
struct Keyframe {
	float time;
	tValue value;
};

using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template <typename tValue>
struct AnimationCurve {
	std::vector<Keyframe<tValue>> keyframes;
};

struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	float duration;
	std::map<std::string, NodeAnimation> nodeAnimations;
};

// スケルトン構造体
struct Joint {
	QuaternionTransform transform; // transform情報
	Matrix4x4 localMatrix; // localMatrix
	Matrix4x4 skeletonSpaceMatrix; // skeletonSpaceでの変換行列
	std::string name; // 名前
	std::vector<int32_t> children; // 子JointのIndexのリスト。いなければ空
	int32_t index; // 自身のIndex
	std::optional<int32_t> parent; // 親JointのIndex。いなければnyll
};

struct Skeleton {
	int32_t root; // RootJointのIndex
	std::map<std::string, int32_t> jointMap; // Joint名とIndexの辞書
	std::vector<Joint> joints; // 所属しているJoint
};

const uint32_t kNumMaxInfluence = 4;
struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix; // 位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix; // 法線用
};

struct SkinCluster {
	std::vector<Matrix4x4> inverseBindPoseMatrices;
	Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
	std::span<VertexInfluence> mappedInfluence;
	Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
	Microsoft::WRL::ComPtr<ID3D12Resource> outputVertexResource;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> outputUavHandle;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> influenceSrvHandle;
	D3D12_VERTEX_BUFFER_VIEW outputVertexBufferView;
	D3D12_RESOURCE_STATES outputVertexState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
};

struct SkinningInformation {
	uint32_t numVertices;
};

enum class ModelRenderType {
	Normal,
	Skinning
};

Vector3 GetMatrix4x4Translate(const Matrix4x4& m);

Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

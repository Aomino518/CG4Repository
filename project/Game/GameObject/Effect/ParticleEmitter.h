#pragma once
#include "Matrix.h"
#include "CreateResorceUtils.h"
#include "ParticleManager.h"

struct ParticleConfig {
	Vector3 minVelocity;
	Vector3 maxVelocity;
	Vector4 startColor;
	Vector4 endColor;
	float minLifeTime;
	float maxLifeTime;
	float startScale;
	float endScale;
};

class ParticleEmitter
{
public:
	/// <summary>
	/// Emitterコンストラクタ
	/// </summary>
	/// <param name="groupName">名称</param>
	/// <param name="count">1回にスポーンする数</param>
	/// <param name="startColor">始めの色</param>
	/// <param name="endColor">終わり色</param>
	/// <param name="startScale">始めのスケール</param>
	/// <param name="endScale">終わりのスケール</param>
	/// <param name="plusRange">正の生成範囲</param>
	/// <param name="minusRange">負の生成範囲</param>
	/// <param name="frequency">スポーン間隔</param>
	ParticleEmitter(const std::string& groupName, 
		uint32_t count, 
		const Vector4& startColor,
		const Vector4& endColor,
		const Vector3& startScale,
		const Vector3& endScale,
		const float plusRange,
		const float minusRange,
		float frequency);
	void EmitOnce();
	void StartLoop();
	void StopLoop();
	void Update();

	// Getter関数
	bool GetIsLoop() { return isLoop_; }
	uint32_t GetCount() const { return count_; }
	float GetFrenquency() const { return frequency_; }
	Transform GetTransform() const { return transform_; }

	// Setter関数
	void SetCount(uint32_t count) { this->count_ = count; }
	void SetFrenquency(float frequency) { this->frequency_ = frequency; }
	void SetTransform(Transform transform) { this->transform_ = transform; }

	Transform transform_;
	uint32_t count_; // 1回の発生個数
	float frequency_; // 発生頻度
	Vector4 startColor_;
	Vector4 endColor_;
	Vector3 startScale_;
	Vector3 endScale_;
	float plusRange_;
	float minusRange_;

private:
	std::string groupName_;
	float frequencyTime_; // 発生タイマー
	const float kDeltaTime = 1.0f / 60.0f;
	bool isLoop_ = false; // 無限発生か
};
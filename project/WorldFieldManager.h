#pragma once
#include "AccelerationField.h"

class WorldFieldManager
{
public:
	static WorldFieldManager* GetInstance();

	void CreateWorldField(
		std::string name, 
		Vector3 position = { 0.0f, 0.0f, 0.0f} ,
		Vector3 acceleration = { 0.0f, 0.0f, 0.0f },
		AABB area = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} },
		bool isActive = true);

	/// <summary>
	/// ワールドフィールド削除関数
	/// </summary>
	/// <param name="name"></param>
	void RemoveField(const std::string& name);

	// Getter関数
	AccelerationField* GetAccelerationField(const std::string& name);
	std::vector<AccelerationField*> GetFields() const;
	const std::unordered_map<std::string, AccelerationField>& GetWorldFields() const { return worldFields_; }

	// Setter関数
	void SetField(std::string name, AccelerationField field) { this->worldFields_[name] = field; }

private:
	// メンバ関数
	WorldFieldManager() = default;
	~WorldFieldManager() = default;
	WorldFieldManager(const WorldFieldManager&) = delete;
	WorldFieldManager& operator=(const WorldFieldManager&) = delete;

	// メンバ変数
	std::unordered_map<std::string, AccelerationField> worldFields_;
};
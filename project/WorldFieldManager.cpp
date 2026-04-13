#include "WorldFieldManager.h"
#include "Logger.h"

WorldFieldManager* WorldFieldManager::GetInstance() {
	static WorldFieldManager instance;
	return &instance;
}

void WorldFieldManager::CreateWorldField(std::string name, Vector3 position, Vector3 acceleration, AABB area, bool isActive)
{
	AccelerationField field;
	field.SetSpace(FieldSpace::World);
	field.SetPosition(position);
	field.SetAcceleration(acceleration);
	field.SetAABB(area);
	field.SetIsActive(isActive);
	worldFields_.emplace(name, std::move(field));
}

void WorldFieldManager::RemoveField(const std::string& name)
{
	auto it = worldFields_.find(name);
	if (it == worldFields_.end()) {
		return;
	}

	worldFields_.erase(it);

	Logger::Write("WorldField removed: " + name);
}

AccelerationField* WorldFieldManager::GetAccelerationField(const std::string& name)
{
	auto it = worldFields_.find(name);
	assert(it != worldFields_.end() && "WorldField not found");
	return &(it->second);
}

std::vector<AccelerationField*> WorldFieldManager::GetFields() const
{
	std::vector<AccelerationField*> result;
	result.reserve(worldFields_.size());
	for (const auto& worldfield : worldFields_) {
		result.push_back(const_cast<AccelerationField*>(&worldfield.second));
	}
	return result;
}

void WorldFieldManager::DrawDebug()
{
	for (auto& field : worldFields_) {
		field.second.DrawDebug(field.second.GetPosition());
	}
}

#pragma once
#include "SeekerEngine.h"
#include "CreateResorceUtils.h"

class LightManager
{
public:
	static LightManager* GetInstance();

	void Init();
	void Update();
	void Shutdown();

	const DirectionalLight& GetDirectionalLight() const { return *dirLight_; }
	const PointLight& GetPointLight() const { return *pointLight_; }
	const SpotLight& GetSpotLight() const { return *spotLight_; }
	const Microsoft::WRL::ComPtr<ID3D12Resource> GetDirLightResource() const { return directionalLightResource_; }
	const Microsoft::WRL::ComPtr<ID3D12Resource> GetPointLightResource() const { return pointLightResource_; }
	const Microsoft::WRL::ComPtr<ID3D12Resource> GetSpotLightResource() const { return spotLightResource_; }

	void SetDirectionalLight(DirectionalLight* dirLight);
	void SetPointLight(PointLight* pointLight);
	void SetSpotLight(SpotLight* spotLight);

private:
	static LightManager* instance_;
	LightManager() = default;
	~LightManager() = default;
	LightManager(const LightManager&) = delete;
	LightManager& operator=(const LightManager&) = delete;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* dirLight_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	PointLight* pointLight_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource_ = nullptr;
	SpotLight* spotLight_ = nullptr;
};


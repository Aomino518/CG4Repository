#include "LightManager.h"

LightManager* LightManager::GetInstance()
{
    static LightManager instance;
    return &instance;
}

void LightManager::Init()
{
    // 平行光源用のリソース
    directionalLightResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(DirectionalLight));
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&dirLight_));
    // 初期化値
    dirLight_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    dirLight_->direction = { 1.0f, 0.0f, 0.0f };
    dirLight_->intensity = 0.0f;

    // ポイントライトのリソース
    pointLightResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(PointLight));
    pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLight_));
    // 初期化値
    pointLight_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    pointLight_->position = { 0.0f, 2.0f, 2.0f };
    pointLight_->intensity = 0.0f;
    pointLight_->radius = 5.0f;
    pointLight_->decay = 1.0f;

    // スポットライトのリリース
    spotLightResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(SpotLight));
    spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLight_));
    spotLight_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    spotLight_->position = { -10.0f, 1.0f, 0.0f };
    spotLight_->intensity = 15.0f;
    spotLight_->decay = 2.0f;
    spotLight_->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
    spotLight_->cosFalloffStart = 1.0f;
    spotLight_->direction = Normalize({ -1.0f, -1.0f, 0.0f });
    spotLight_->distance = 20.0f;
}

void LightManager::Update()
{
    
}

void LightManager::Shutdown()
{
    directionalLightResource_->Release();
    pointLightResource_->Release();
    spotLightResource_->Release();
}

void LightManager::SetDirectionalLight(DirectionalLight* dirLight)
{
    dirLight_->direction = dirLight->direction;
    dirLight_->color = dirLight->color;
    dirLight_->intensity = dirLight->intensity;
}

void LightManager::SetPointLight(PointLight* pointLight)
{
    pointLight_->position = pointLight->position;
    pointLight_->color = pointLight->color;
    pointLight_->intensity = pointLight->intensity;
    pointLight_->radius = pointLight->radius;
    pointLight_->decay = pointLight->decay;
}

void LightManager::SetSpotLight(SpotLight* spotLight)
{
    spotLight_->position = spotLight->position;
    spotLight_->color = spotLight->color;
    spotLight_->intensity = spotLight->intensity;
    spotLight_->distance = spotLight->distance;
    spotLight_->decay = spotLight->decay;
    spotLight_->direction = spotLight->direction;
    spotLight_->cosAngle = spotLight->cosAngle;
    spotLight_->cosFalloffStart = spotLight->cosFalloffStart;
}

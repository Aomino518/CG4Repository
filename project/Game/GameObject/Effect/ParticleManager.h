#pragma once
#include "DxcCompiler.h"
#include <random>
#include <numbers>
#include "SrvManager.h"
#include "TextureManager.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "CreateResorceUtils.h"
#include "CameraManager.h"
#include "BlendStateUtils.h"

struct AccelerationField {
    Vector3 acceleration;
    AABB area;
};

struct ParticleGroup {
    // テクスチャ関連
    uint32_t textureIndex = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandle{};
    // パーティクル本体
    std::list<Particle> particles;
    // インスタンシング
    Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource;
    uint32_t instanceCount = 0;
    uint32_t srvIndexCount = 0;
    ParticleForGPU* instanceData = nullptr;
    // ビルボードを使用しているか
    bool useBillboard_ = false;
    // ブレンドモード
    BlendMode blendMode_ = kBlendModeNone;
    // 風を使用してるか
    bool useField_ = false;
    // フィールド
    AccelerationField field_ = {
      { 0.0f, 0.0f, 0.0f },
      {{-1.0f, -1.0f, -1.0f},
      { 1.0f,  1.0f,  1.0f}}
    };
};

class Graphics;
class ParticleManager
{
public:
	static ParticleManager* GetInstance();

	void Init(DxcCompiler& dxcCompiler, ID3D12RootSignature* rootSignature);

	void Update(CameraManager* cameraManager);

	void Draw();

    void Shutdown();

    void Emit(const std::string name, 
        const Vector3& position, 
        const Vector4& startColor,
        const Vector4& endColor,
        const Vector3& startScale,
        const Vector3& endScale,
        const float plusRange,
        const float minusRange,
        uint32_t count);

    void SetBlendMode(const std::string& name, BlendMode mode);

    void RebuildPso();

    // パーティクルグループの生成
    void CreateParticleGroup(const std::string& name, uint32_t textureId);

    // パーティクルグループの削除
    void RemoveParticleGroup(const std::string& name);

    // Getter関数
    ParticleGroup& GetGroup(const std::string& name) { return particleGroups[name]; }
    bool GetUseBillboard(const std::string& name) { return particleGroups[name].useBillboard_; }
    bool GetUseField(const std::string& name) { return particleGroups[name].useField_; }
    BlendMode GetBlendMode(const std::string& name);
    uint32_t GetkNumMaxInstance() { return kNumMaxInstance_; }
    AccelerationField& GetField(const std::string& name) { return particleGroups[name].field_; }
    ID3D12PipelineState* GetPso(BlendMode mode);
   
    void SetUseBillboard(const std::string& name, bool useBillboard) { this->particleGroups[name].useBillboard_ = useBillboard; }
    void SetUseField(const std::string& name, bool useField) { this->particleGroups[name].useField_ = useField; }

private:
    ParticleManager() = default;
    ~ParticleManager() = default;
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

    // グラフィックパイプラインを生成
    void CreateGraphicsPipeline(DxcCompiler& dxcCompiler);
    // 板ポリの生成関数
    void CreatePlaneModel();
    // ワールド計算行列
    Matrix4x4 CalculateWorldMatrix(const Particle& particle, const std::string& name, bool isDebug);
    // WVP計算行列
    Matrix4x4 CalculateWVPMatrix(const Matrix4x4& worldMatrix, bool isDebug);

    Graphics* graphics_ = nullptr;
    Camera* camera_ = nullptr;
    DebugCamera* debugCamera_ = nullptr;
    CameraManager* cameraManager_ = nullptr;

    // パーティクルグループコンテナ
    std::unordered_map<std::string, ParticleGroup> particleGroups;
    // ランダムエンジンの変数
    std::mt19937 randomEngine_;
    static constexpr uint32_t kNumMaxInstance_ = 1000;
    const float kDeltaTime = 1.0f / 60.0f;
   
    //===========================
    // グラフィックパイプラインの変数
    //===========================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psoParticle3D_;

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;

    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
    Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;

    D3D12_BLEND_DESC blendDesc_{};
    BlendMode mode_ = kBlendModeNone;
    std::unordered_map<BlendMode, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;

    // リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    D3D12_INDEX_BUFFER_VIEW  ibView_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
    Material* materialData = nullptr;
};


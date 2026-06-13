#include "TitleScene.h"
#include "SceneIncludes.h"
#include "Skybox.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");

    tHTex_ = TextureManager::GetInstance()->Load("./resources/rostock_laage_airport_4k.dds");
    tHCircle_ = TextureManager::GetInstance()->Load("./resources/sprites/circle2.png");
    tHGradationLine_ = TextureManager::GetInstance()->Load("./resources/sprites/gradationLine.png");
    Skybox::GetInstance()->SetTexture(tHTex_);

    entity_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball.obj");
    ModelManager::GetInstance()->FindModel("ball")->SetEnviromentTexture(tHTex_);
    entity_->Init();
    entity_->SetModel("ball");
    entity_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("ball", entity_.get());

    modelTerrain_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    ModelManager::GetInstance()->FindModel("terrain")->SetEnviromentTexture(tHTex_);
    modelTerrain_->Init();
    modelTerrain_->SetModel("terrain");
    modelTerrain_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("terrain", modelTerrain_.get());

    ParticleManager::GetInstance()->CreateParticleGroup("HitEffect", tHCircle_);
    EmitterManager::GetInstance()->CreateEmitter("HitEffect", hitEffectConfig_);
    Editor::GetInstance()->RegisterParticle("HitEffect");

    ParticleManager::GetInstance()->CreateParticleGroup("RingEffect", tHGradationLine_);
    ParticleManager::GetInstance()->SetModelType("RingEffect", ParticleModelType::Ring);
    EmitterManager::GetInstance()->CreateEmitter("RingEffect", hitRingEffectConfig_);
    Editor::GetInstance()->RegisterParticle("RingEffect");

    ParticleManager::GetInstance()->CreateParticleGroup("CylinderEffect", tHGradationLine_);
    ParticleManager::GetInstance()->SetModelType("CylinderEffect", ParticleModelType::Cylinder);
    EmitterManager::GetInstance()->CreateEmitter("CylinderEffect", cylinderEffectConfig_);
    Editor::GetInstance()->RegisterParticle("CylinderEffect");

    ImGuiManager::GetInstance()->LoadScenesJson();
}

void TitleScene::Update()
{
    auto camMgr = CameraManager::GetInstance();

    if (Input::GetInstance()->IsTrigger(DIK_ESCAPE)) {
        EndRequset();
    }

    if (Input::GetInstance()->IsTrigger(DIK_SPACE)) {
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    if (Input::GetInstance()->IsTrigger(DIK_R)) {
        auto hitEffectTransform = EmitterManager::GetInstance()->GetEmitter("HitEffect")->GetTransform();
        auto hitEffectCount = EmitterManager::GetInstance()->GetEmitter("HitEffect")->GetCount();
        hitEffectConfig_ = EmitterManager::GetInstance()->GetEmitter("HitEffect")->GetConfig();
        auto ringEffectTransform = EmitterManager::GetInstance()->GetEmitter("RingEffect")->GetTransform();
        auto ringEffectCount = EmitterManager::GetInstance()->GetEmitter("RingEffect")->GetCount();
        hitRingEffectConfig_ = EmitterManager::GetInstance()->GetEmitter("RingEffect")->GetConfig();
        ParticleManager::GetInstance()->Emit("HitEffect", hitEffectConfig_, hitEffectTransform.translate, hitEffectCount);
        ParticleManager::GetInstance()->Emit("RingEffect", hitRingEffectConfig_, ringEffectTransform.translate, ringEffectCount);
    }

    Skybox::GetInstance()->Update();
    entity_->SetCamera(camMgr->GetActiveCamera());
    entity_->Update();
    modelTerrain_->SetCamera(camMgr->GetActiveCamera());
    modelTerrain_->Update();

    EmitterManager::GetInstance()->Update();
    ParticleManager::GetInstance()->Update(camMgr);

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
    Skybox::GetInstance()->Draw();
    entity_->Draw();
    modelTerrain_->Draw();
    ParticleManager::GetInstance()->Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    ParticleManager::GetInstance()->RemoveParticleGroup("HitEffect");
    EmitterManager::GetInstance()->RemoveEmitter("HitEffect");
    ParticleManager::GetInstance()->RemoveParticleGroup("RingEffect");
    EmitterManager::GetInstance()->RemoveEmitter("RingEffect");
    ParticleManager::GetInstance()->RemoveParticleGroup("CylinderEffect");
    EmitterManager::GetInstance()->RemoveEmitter("CylinderEffect");
    Editor::GetInstance()->Clear();
}

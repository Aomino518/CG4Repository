#include "TitleScene.h"
#include "SceneIncludes.h"
#include "Skybox.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");

    tHTex_ = TextureManager::GetInstance()->Load("./resources/rostock_laage_airport_4k.dds");
    Skybox::GetInstance()->SetTexture(tHTex_);

    entity_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball.obj");
    entity_->Init();
    entity_->SetModel("ball");
    entity_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("ball", entity_.get());

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

    Skybox::GetInstance()->Update();
    entity_->SetCamera(camMgr->GetActiveCamera());
    entity_->Update();

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
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    Editor::GetInstance()->Clear();
}

#include "TitleScene.h"
#include "SceneIncludes.h"
#include "Skybox.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");
   
    tHTex_ = TextureManager::GetInstance()->Load("./resources/rostock_laage_airport_4k.dds");
    Skybox::GetInstance()->SetTexture(tHTex_);
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
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    Editor::GetInstance()->Clear();
}

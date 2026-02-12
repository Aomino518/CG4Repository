#include "TitleScene.h"
#include "SceneManager.h"

void TitleScene::Init()
{
    //===========================
    // Sound
    //===========================
    SoundManager::GetInstance()->Load("bgm1", "resources/sound5.wav");

    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/uvChecker.png");
    sprite->Create(tHChecker, { 0.0f, 0.0f }, Color::WHITE);
    sprite->SetRotation(0.0f);
}

void TitleScene::Update()
{
    if (Input::GetInstance()->IsPressed(DIK_M)) {
        SoundManager::GetInstance()->PlayBGM("bgm1");
    }

    if (Input::GetInstance()->IsPressed(DIK_N)) {
        SoundManager::GetInstance()->StopBGM();
    }

    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
       SoundManager::GetInstance()->StopBGM();
       SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    SoundManager::GetInstance()->Update();

    sprite->Update();

    ImGuiManager::GetInstance()->BegineFrame();
    ImGuiManager::GetInstance()->BegineInspector();
    ImGuiManager::GetInstance()->SpriteSetting("uvChecker", sprite.get());
    ImGuiManager::GetInstance()->EndInspector();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
    SpriteCommon::GetInstance()->DrawCommon();
    sprite->Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{

}

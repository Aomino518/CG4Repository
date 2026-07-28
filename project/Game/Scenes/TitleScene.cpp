#include "TitleScene.h"
#include "SceneIncludes.h"
#include "Skybox.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");

    tHTex_ = TextureManager::GetInstance()->Load("./resources/rostock_laage_airport_4k.dds");
    Skybox::GetInstance()->SetTexture(tHTex_);

    modelTerrain_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain", "terrain.obj");
    ModelManager::GetInstance()->FindModel("terrain")->SetEnviromentTexture(tHTex_);
    modelTerrain_->Init("terrain", false);
    modelTerrain_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("terrain", modelTerrain_.get());

    simpleSkin_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("human", "sneakWalk.gltf");
    ModelManager::GetInstance()->FindModel("sneakWalk")->SetEnviromentTexture(tHTex_);
    simpleSkin_->Init("sneakWalk", true);
    simpleSkin_->SetTranslate(Vector3{ -2.0f, 0.0f, 2.0f });
    Editor::GetInstance()->RegisterModel("sneakWalk", simpleSkin_.get());

    ImGuiManager::GetInstance()->LoadScenesJson();
    CameraManager::GetInstance()->SetActiveCameraByName("MainCamera");
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

    UpdatePlayer();
    ResolveTerrainCollision();
    UpdateThirdPersonCamera();

    Skybox::GetInstance()->Update();
    modelTerrain_->SetCamera(camMgr->GetActiveCamera());
    modelTerrain_->Update();
    simpleSkin_->SetCamera(camMgr->GetActiveCamera());
    simpleSkin_->Update();

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
    modelTerrain_->Draw();
    simpleSkin_->Draw();

    simpleSkin_->DrawBorn();
    DebugDraw::Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    Editor::GetInstance()->Clear();
}

void TitleScene::UpdatePlayer() {
    auto input = Input::GetInstance();
    Vector2 stick = input->GetXbLeftStickVector();

    if (input->IsPress(DIK_A)) {
        stick.x -= 1.0f;
    }
    if (input->IsPress(DIK_D)) {
        stick.x += 1.0f;
    }
    if (input->IsPress(DIK_S)) {
        stick.y -= 1.0f;
    }
    if (input->IsPress(DIK_W)) {
        stick.y += 1.0f;
    }

    // 斜めが速くならないように長さを1に正規化
    const float inputLength = std::sqrt(stick.x * stick.x + stick.y * stick.y);
    if (inputLength > 1.0f) {
        stick.x /= inputLength;
        stick.y /= inputLength;
    }
    const bool isMoving = inputLength > 0.0f;

    simpleSkin_->SetAnimationPlaying(isMoving);
    if (!isMoving) {
        return;
    }

    Vector3 position = simpleSkin_->GetTranslate();
    position.x += stick.x * moveSpeed_;
    position.z += stick.y * moveSpeed_;
    simpleSkin_->SetTranslate(position);

    Vector3 rotation = simpleSkin_->GetRotate();
    rotation.y = std::atan2(stick.x, stick.y);
    simpleSkin_->SetRotate(rotation);
}

void TitleScene::ResolveTerrainCollision()
{
    const float terrainSurfaceY = modelTerrain_->GetTranslate().y;
    Vector3 position = simpleSkin_->GetTranslate();
    const float playerBottom = position.y - playerHalfHeight_;

    if (playerBottom < terrainSurfaceY) {
        position.y = terrainSurfaceY + playerHalfHeight_;
        simpleSkin_->SetTranslate(position);
    }
}

void TitleScene::UpdateThirdPersonCamera()
{
    Camera* mainCamera = CameraManager::GetInstance()->GetCamera("MainCamera");
    if (!mainCamera) {
        return;
    }

    const Vector3 playerPosition = simpleSkin_->GetTranslate();
    mainCamera->SetTranslate({
        playerPosition.x + cameraOffset_.x,
        playerPosition.y + cameraOffset_.y,
        playerPosition.z + cameraOffset_.z
        });

    mainCamera->SetRotate({ cameraPitch_, 0.0f, 0.0f });
}

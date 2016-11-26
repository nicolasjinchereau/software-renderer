/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include <windows.h>
#include <Windowsx.h>
#include <SDKDDKVer.h>
#include <stdlib.h>
#include <memory>
#include "RenderingContext.h"
#include "Texture.h"
#include "Time.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "SceneObject.h"
#include "Scene.h"
#include "Mem.h"
#include "Application.h"
#include "CustomShaders.h"
#include <thread>
using namespace std;

// Controls:
//   W:    forward
//   A:    left
//   S:    back
//   D:    right
//   Q:    up
//   E:    down
//   LMB:  mouse look
//   T:    cycle tex filter (point, bilinear, trilinear)
//   M:    toggle mipmaps
//   L:    toggle lighting
//   F:    cycle antialiasing (None, 4x MSAA, 2x SSAA, 4x SSAA)
//   C:    toggle framerate cap
//   R:    reload scene_settings.json

class RenderingApp : public Application
{
    constexpr static int screenWidth = 640;
    constexpr static int screenHeight = 480;
    //constexpr static int screenWidth = 1024;
    //constexpr static int screenHeight = 768;
    //constexpr static int screenWidth = 800;
    //constexpr static int screenHeight = 500;
    //constexpr static int screenWidth = 1280;
    //constexpr static int screenHeight = 800;

    const float xRotationSpeed = 0.5f;
    const float yRotationSpeed = 0.5f;
    const float maxSpeed = 7.0f;
    const float accelTime = 0.5f;
    const float accelRate = maxSpeed / accelTime;
    const float decelTime = 0.25f;
    const float decelRate = maxSpeed / decelTime;
    const FilterMode defaultFilterMode = FilterMode::Bilinear;
    const float maxFramerate = 30;
    const float minFrameInterval = 1.0f / maxFramerate;
    
    shared_ptr<RenderingContext> context;
    vector<shared_ptr<Texture>> textures;
    shared_ptr<UnlitShader> unlitShader;
    shared_ptr<LitShader> litShader;
    shared_ptr<LitCutoutShader> litCutoutShader;
    shared_ptr<Scene> scene;

    shared_ptr<Texture> skyDayTex;
    shared_ptr<Texture> skyNightTex;

    bool fwdKeyDown = false;
    bool backKeyDown = false;
    bool leftKeyDown = false;
    bool rightKeyDown = false;
    bool upKeyDown = false;
    bool downKeyDown = false;
    bool mouseLook = false;
    float mouseX = 0;
    float mouseY = 0;
    float xAngle = 0;
    float yAngle = 0;
    FilterMode filterMode = defaultFilterMode;
    float speed = 0;
    Vec3 inputDir = Vec3::zero;
    float lastUpdate = 0;
    bool capFramerate = false;

public:
    RenderingApp() : Application("Software Renderer", screenWidth, screenHeight){}

    virtual void OnInitialize() override
    {
        // initialize rendering context
        size_t threadCount = thread::hardware_concurrency();
        context = AlignedMakeShared<RenderingContext, 16>(this, screenWidth, screenHeight, threadCount);
        context->clearColor(Color::clear);
        context->rasterizationMode(RasterizationMode::Halfspace);
        context->mipmapsEnabled(true);

        // create shaders
        unlitShader = AlignedMakeShared<UnlitShader, 16>();
        litShader = AlignedMakeShared<LitShader, 16>();
        litCutoutShader = AlignedMakeShared<LitCutoutShader, 16>();

        // load textures
        auto terrainTex = AlignedMakeShared<Texture, 16>("textures/terrain.tga", filterMode);
        auto houseTex = AlignedMakeShared<Texture, 16>("textures/house.tga", filterMode);
        auto house2Tex = AlignedMakeShared<Texture, 16>("textures/house2.tga", filterMode);
        auto plantsTex = AlignedMakeShared<Texture, 16>("textures/plants.tga", filterMode);
        auto carTex = AlignedMakeShared<Texture, 16>("textures/delorean.tga", filterMode);
        auto lampTex = AlignedMakeShared<Texture, 16>("textures/lamp.tga", filterMode);
        auto rockTex = AlignedMakeShared<Texture, 16>("textures/rock.tga", filterMode);
        auto yuccaTreeTex = AlignedMakeShared<Texture, 16>("textures/yuccaTree.tga", filterMode);
        skyDayTex = AlignedMakeShared<Texture, 16>("textures/skyDay.tga", filterMode);
        skyNightTex = AlignedMakeShared<Texture, 16>("textures/skyNight.tga", filterMode);
        textures.push_back(terrainTex);
        textures.push_back(houseTex);
        textures.push_back(house2Tex);
        textures.push_back(carTex);
        textures.push_back(lampTex);
        textures.push_back(rockTex);
        textures.push_back(yuccaTreeTex);
        textures.push_back(skyDayTex);
        textures.push_back(skyNightTex);

        // load models
        auto terrainModel = AlignedMakeShared<Model, 16>("meshes/terrain.fbx");
        auto houseModel = AlignedMakeShared<Model, 16>("meshes/house.fbx");
        auto house2Model = AlignedMakeShared<Model, 16>("meshes/house2.fbx");
        auto plantsModel = AlignedMakeShared<Model, 16>("meshes/plants.fbx");
        auto carModel = AlignedMakeShared<Model, 16>("meshes/delorean.fbx");
        auto lampModel = AlignedMakeShared<Model, 16>("meshes/lamp.fbx");
        auto rockModel = AlignedMakeShared<Model, 16>("meshes/rock.fbx");
        auto yuccaTreeModel = AlignedMakeShared<Model, 16>("meshes/yuccaTree.fbx");
        auto skyModel = AlignedMakeShared<Model, 16>("meshes/sky.fbx");
        
        // create camera
        xAngle = 1.0f;
        yAngle = 124.0f;
        shared_ptr<Camera> cam = AlignedMakeShared<Camera, 16>(60.0f, (float)clientWidth() / (float)clientHeight(), 0.1f, 300.0f);
        cam->transform.SetPosition(-13.8f, 1.6f, 9.0f);
        cam->transform.SetRotation(xAngle, yAngle, 0.0f);
        
        // create scene objects
        auto houseObj = AlignedMakeShared<SceneObject, 16>("house", houseModel, houseTex, litShader);
        auto house2Obj = AlignedMakeShared<SceneObject, 16>("house2", house2Model, house2Tex, litShader);
        auto plants1Obj = AlignedMakeShared<SceneObject, 16>("plants1", plantsModel, plantsTex, litCutoutShader, CullMode::None);
        auto plants2Obj = AlignedMakeShared<SceneObject, 16>("plants2", plantsModel, plantsTex, litCutoutShader, CullMode::None);
        auto plants3Obj = AlignedMakeShared<SceneObject, 16>("plants3", plantsModel, plantsTex, litCutoutShader, CullMode::None);
        auto carObj = AlignedMakeShared<SceneObject, 16>("car", carModel, carTex, litShader);
        auto lampObj = AlignedMakeShared<SceneObject, 16>("lamp", lampModel, lampTex, litShader);
        auto rockObj = AlignedMakeShared<SceneObject, 16>("rock", rockModel, rockTex, litShader);
        auto yuccaTreeObj1 = AlignedMakeShared<SceneObject, 16>("yucca1", yuccaTreeModel, yuccaTreeTex, litShader, CullMode::None);
        auto yuccaTreeObj2 = AlignedMakeShared<SceneObject, 16>("yucca2", yuccaTreeModel, yuccaTreeTex, litShader, CullMode::None);
        auto terrainObj = AlignedMakeShared<SceneObject, 16>("terrain", terrainModel, terrainTex, litShader);
        auto skyObj = AlignedMakeShared<SceneObject, 16>("sky", skyModel, skyNightTex, unlitShader);
        
        // create scene lights
        auto ambient = AlignedMakeShared<AmbientLight, 16>("ambient_light", Color32(118, 173, 218, 255), 0.4f);
        auto direct = AlignedMakeShared<DirectionalLight, 16>("direct_light");
        auto lampLight = AlignedMakeShared<PointLight, 16>("lamp_light");
        auto ltHeadlight = AlignedMakeShared<SpotLight, 16>("left_headlight");
        auto rtHeadlight = AlignedMakeShared<SpotLight, 16>("right_headlight");

        // assemble scene and apply settings
        scene = AlignedMakeShared<Scene, 16>();
        scene->camera = cam;
        scene->objects.push_back(houseObj);
        scene->objects.push_back(house2Obj);
        scene->objects.push_back(plants1Obj);
        scene->objects.push_back(plants2Obj);
        scene->objects.push_back(plants3Obj);
        scene->objects.push_back(carObj);
        scene->objects.push_back(lampObj);
        scene->objects.push_back(rockObj);
        scene->objects.push_back(yuccaTreeObj1);
        scene->objects.push_back(yuccaTreeObj2);
        scene->objects.push_back(terrainObj);
        scene->objects.push_back(skyObj);
        scene->lights.push_back(ambient);
        scene->lights.push_back(direct);
        scene->lights.push_back(lampLight);
        scene->lights.push_back(ltHeadlight);
        scene->lights.push_back(rtHeadlight);
        scene->ApplySettings("scene/scene_settings.json");
    }
    
    uint32_t lastFps = 0;

    virtual bool OnUpdate() override
    {
        if(capFramerate)
        {
            float now = Time::time();
            float elapsed = now - lastUpdate;
            
            if(elapsed < minFrameInterval)
            {
                SleepFor(minFrameInterval - elapsed);
                return true;
            }

            lastUpdate = now;
        }

        UpdateCamera();

        context->Clear(false, true);
        context->Draw(scene);
        context->Present();

        Time::update();

        auto fps = Time::fps();
        if(fps != lastFps)
        {
            lastFps = fps;
            setWindowTitle(makeTitle());
        }

        return true;
    }

    std::string makeTitle()
    {
        static const char* aaModes[]{
            "Off",
            "4X MSAA",
            "2X SSAA",
            "4X SSAA",
        };
        
        static const char* filterModes[]{
            "Point",
            "Bilinear",
            "Trilinear",
        };
        
        static const char* offOn[]{
            "Off",
            "On",
        };

        const char* aaMode = aaModes[(int)context->antiAliasingMode()];
        const char *filtMode = filterModes[(int)filterMode];
        const char* mipmaps = offOn[context->mipmapsEnabled() ? 1 : 0];
        
        char buff[256];
        sprintf_s(buff, "%ux%u - Tex Filter: %s - Mipmaps: %s - AA: %s - FPS: %u",
                  context->width(), context->height(), filtMode, mipmaps, aaMode, lastFps);
        return buff;
    }

    void UpdateCamera()
    {
        scene->camera->transform.SetRotation(xAngle, yAngle, 0);
        Quat camRotation = scene->camera->transform.GetRotation();

        bool anyKeyDown = fwdKeyDown || backKeyDown || leftKeyDown || rightKeyDown || downKeyDown || upKeyDown;

        if(anyKeyDown)
        {
            inputDir = Vec3::zero;
            if(backKeyDown)  inputDir.z -= 1.0f;
            if(fwdKeyDown)   inputDir.z += 1.0f;
            if(leftKeyDown)  inputDir.x -= 1.0f;
            if(rightKeyDown) inputDir.x += 1.0f;
            if(downKeyDown)  inputDir.y -= 1.0f;
            if(upKeyDown)    inputDir.y += 1.0f;
        }

        Vec3 direction = inputDir * camRotation;
        direction.Normalize();
        
        float deltaTime = Time::deltaTime();

        if(anyKeyDown)
            speed += accelRate * deltaTime;
        else
            speed -= decelRate * deltaTime;

        speed = Math::Clamp(speed, 0.0f, maxSpeed);

        Vec3 velocity = Vec3::zero;
        if(speed > 0.0000001f)
            velocity = direction * speed;

        Vec3 camPos = scene->camera->transform.GetPosition();
        camPos += velocity * deltaTime;
        scene->camera->transform.SetPosition(camPos);
    }

    virtual void OnKeyDown(KeyCode key) override {
        switch(key)
        {
        case KeyCode::T:
            if(filterMode == FilterMode::Point)
                SetTextureFilters(FilterMode::Bilinear);
            else if(filterMode == FilterMode::Bilinear)
                SetTextureFilters(FilterMode::Trilinear);
            else
                SetTextureFilters(FilterMode::Point);
            break;

        case KeyCode::M:
            context->mipmapsEnabled( !context->mipmapsEnabled() );
            break;

        case KeyCode::L:
            litShader->enableLighting = !litShader->enableLighting;
            litCutoutShader->enableLighting = litShader->enableLighting;

            scene->FindObject("sky")->texture = litShader->enableLighting?
                skyNightTex : skyDayTex;

            break;

        case KeyCode::C:
            capFramerate = !capFramerate;
            break;

        case KeyCode::R:
            scene->ApplySettings("scene/scene_settings.json");
            break;

        case KeyCode::F:
            if(context->antiAliasingMode() == AntiAliasingMode::Off)
                context->antiAliasingMode(AntiAliasingMode::MSAA_4X);
            else if(context->antiAliasingMode() == AntiAliasingMode::MSAA_4X)
                context->antiAliasingMode(AntiAliasingMode::SSAA_2X);
            else if(context->antiAliasingMode() == AntiAliasingMode::SSAA_2X)
                context->antiAliasingMode(AntiAliasingMode::SSAA_4X);
            else
                context->antiAliasingMode(AntiAliasingMode::Off);

            break;

        case KeyCode::Space:

            break;
        
        default:
            SetKeyState(key, true);
            break;
        }
    }

    virtual void OnKeyUp(KeyCode key) override {
        SetKeyState(key, false);
    }

    virtual void OnPointerDown(float x, float y, int id) override {
        if(id == 0)
        {
            mouseLook = true;
            mouseX = x;
            mouseY = y;
        }
    }

    virtual void OnPointerMove(float x, float y, int id) override
    {
        if(mouseLook)
        {
            float dx = x - mouseX;
            float dy = y - mouseY;
            mouseX = x;
            mouseY = y;

            xAngle += dy * yRotationSpeed;
            yAngle += dx * xRotationSpeed;
            yAngle = fmod(yAngle, 360.0f);
        }
    }

    virtual void OnPointerUp(float x, float y, int id) override {
        if(id == 0)
            mouseLook = false;
    }

    void SetKeyState(KeyCode keycode, bool isDown)
    {
        switch(keycode)
        {
        case KeyCode::W:
            fwdKeyDown = isDown;
            break;

        case KeyCode::S:
            backKeyDown = isDown;
            break;

        case KeyCode::A:
            leftKeyDown = isDown;
            break;

        case KeyCode::D:
            rightKeyDown = isDown;
            break;

        case KeyCode::Q:
            downKeyDown = isDown;
            break;

        case KeyCode::E:
            upKeyDown = isDown;
            break;
        }
    }

    void SetTextureFilters(FilterMode mode)
    {
        filterMode = mode;
        for(auto& tex : textures)
            tex->filterMode(mode);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    RenderingApp app;
    return app.Run();
}

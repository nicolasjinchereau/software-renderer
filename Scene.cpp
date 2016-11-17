/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Scene.h"
#include "Json.h"
#include <exception>
#include <string>

void Scene::ApplySettings(const string& filename)
{
    try {
        JSONObject settings = JSONParser::Load(filename);
        for(auto& obj : settings)
        {
            auto so = FindObject(obj.first);
            if(so)
            {
                JSONValue& data = obj.second;

                auto& pos = data["pos"];
                auto& rot = data["rot"];

                so->transform.SetPosition(Vec3(pos[0], pos[1], pos[2]));
                so->transform.SetRotation(Vec3(rot[0], rot[1], rot[2]));
            }

            auto lightObj = FindLight(obj.first);
            if(lightObj)
            {
                JSONValue& data = obj.second;

                LightType type = lightObj->type();

                if(type == LightType::Ambient)
                {
                    auto light = (AmbientLight*)lightObj.get();
                    auto& color = data["color"];
                    auto& multiplier = data["multiplier"];

                    light->color = Color(color[0], color[1], color[2], color[3]);
                    light->intensity = multiplier;
                }
                else if(type == LightType::Directional)
                {
                    auto light = (DirectionalLight*)lightObj.get();

                    auto& color = data["color"];
                    auto& multiplier = data["multiplier"];
                    auto& dir = data["dir"];

                    light->color = Color(color[0], color[1], color[2], color[3]);
                    light->intensity = multiplier;
                    light->direction = Vec3(dir[0], dir[1], dir[2]);
                }
                else if(type == LightType::Point)
                {
                    auto light = (PointLight*)lightObj.get();

                    auto& color = data["color"];
                    auto& multiplier = data["multiplier"];
                    auto& pos = data["pos"];
                    auto& distAttenMin = data["distAttenMin"];
                    auto& distAttenMax = data["distAttenMax"];

                    light->color = Color(color[0], color[1], color[2], color[3]);
                    light->intensity = multiplier;
                    light->position = Vec3(pos[0], pos[1], pos[2]);
                    light->distAttenMin = distAttenMin;
                    light->distAttenMax = distAttenMax;
                }
                else if(type == LightType::Spot)
                {
                    auto light = (SpotLight*)lightObj.get();

                    auto& color = data["color"];
                    auto& multiplier = data["multiplier"];
                    auto& pos = data["pos"];
                    auto& dir = data["dir"];
                    auto& angAttenMin = data["angAttenMin"];
                    auto& angAttenMax = data["angAttenMax"];
                    auto& distAttenMin = data["distAttenMin"];
                    auto& distAttenMax = data["distAttenMax"];
                    
                    light->color = Color(color[0], color[1], color[2], color[3]);
                    light->intensity = multiplier;
                    light->position = Vec3(pos[0], pos[1], pos[2]);
                    light->direction = Vec3(dir[0], dir[1], dir[2]);
                    light->angAttenMin = angAttenMin;
                    light->angAttenMax = angAttenMax;
                    light->distAttenMin = distAttenMin;
                    light->distAttenMax = distAttenMax;
                }
            }
        }
    }
    catch(std::exception& ex) {
        MessageBox(0, ex.what(), "Error loading scene settings", MB_OK | MB_ICONERROR);
    }
}

shared_ptr<SceneObject> Scene::FindObject(const string& name)
{
    auto it = find_if(objects.begin(), objects.end(), [&name](auto& so){
        return so->name == name;
    });
    return it != objects.end() ? *it : shared_ptr<SceneObject>();
}

shared_ptr<Light> Scene::FindLight(const string& name)
{
    auto it = find_if(lights.begin(), lights.end(), [&name](auto& light){
        return light->name == name;
    });
    return it != lights.end() ? *it : shared_ptr<Light>();
}

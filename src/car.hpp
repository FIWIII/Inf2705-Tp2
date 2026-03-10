#pragma once
#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>           
#include "model.hpp"
#include <textures.hpp>


class EdgeEffect;
class CelShading;
class UniformBuffer;

class Car
{   
public:
    Car();
    
    void loadModels();
    
    void update(float deltaTime);
    
    void draw(glm::mat4& projView, glm::mat4& view);

    void drawWindows(glm::mat4& projView, glm::mat4& view);

    void setColorMod(const glm::vec3& color);
    
private:
    void drawOutlinedModel(const Model& model, glm::mat4& projView, glm::mat4& view, glm::mat4 modelMatrix);
    void drawFrame(glm::mat4& projView, glm::mat4 carModel, glm::mat4& view);

    void drawWheel(glm::mat4& projView, glm::mat4 wheelModel, bool isFrontWheel, bool isLeftWheel, glm::mat4& view);
    void drawWheels(glm::mat4& projView, glm::mat4 carModel, glm::mat4& view);

    void drawBlinker(glm::mat4& projView, glm::mat4 headlightModel, bool isLeftHeadlight, glm::mat4& view);
    void drawLight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight, glm::mat4& view);
    void drawHeadlight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight, bool isLeftHeadlight, glm::mat4& view);
    void drawHeadlights(glm::mat4& projView, glm::mat4 frameModel, glm::mat4& view);

    glm::vec3 lastColorMod_;
    
private:    
    Model frame_;
    Model wheel_;
    Model blinker_;
    Model light_;
    Model windows[6];
    
public:
    glm::vec3 position;
    glm::vec2 orientation;    
    
    float speed;
    float wheelsRollAngle;
    float steeringAngle;
    bool isHeadlightOn;
    bool isBraking;
    bool isLeftBlinkerActivated;
    bool isRightBlinkerActivated;
    
    bool isBlinkerOn;
    float blinkerTimer;

    Texture2D* carTexture = nullptr;
    Texture2D* carWindowTexture = nullptr;
    Texture2D* lightTexture = nullptr;

    glm::mat4 carModel{ 1.0f };

    EdgeEffect* edgeEffectShader{ nullptr };
    CelShading* celShadingShader{ nullptr };
    UniformBuffer* material{ nullptr };
    
    GLuint colorModUniformLocation{0};
    GLuint mvpUniformLocation{0};
};



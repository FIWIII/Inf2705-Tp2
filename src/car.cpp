#include "car.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <map>
#include "shaders.hpp"
#include "uniform_buffer.hpp"

struct Material
{
    glm::vec4 emission; // vec3, but padded
    glm::vec4 ambient;  // vec3, but padded
    glm::vec4 diffuse;  // vec3, but padded
    glm::vec3 specular;
    GLfloat shininess;
};


using namespace gl;
using namespace glm;

    
Car::Car()
    : position(0.0f, 0.0f, 0.0f)
    , orientation(0.0f, 0.0f)
    , speed(0.f)
    , wheelsRollAngle(0.f)
    , steeringAngle(0.f)
    , isHeadlightOn(false)
    , isBraking(false)
    , isLeftBlinkerActivated(false)
    , isRightBlinkerActivated(false)
    , isBlinkerOn(false)
    , blinkerTimer(0.f)
    , lastColorMod_(-1.0f, -1.0f, -1.0f)
{
}


void Car::setColorMod(const glm::vec3& color)
{
    lastColorMod_ = color;
}

void Car::loadModels()
{
    frame_.load("../models/frame.ply");
    wheel_.load("../models/wheel.ply");
    blinker_.load("../models/blinker.ply");
    light_.load("../models/light.ply");

    const char* WINDOW_MODEL_PATHES[] =
    {
        "../models/window.f.ply",
        "../models/window.r.ply",
        "../models/window.fl.ply",
        "../models/window.fr.ply",
        "../models/window.rl.ply",
        "../models/window.rr.ply"
    };
    for (unsigned int i = 0; i < 6; ++i)
    {
        windows[i].load(WINDOW_MODEL_PATHES[i]);
    }
}

void Car::update(float deltaTime)
{
    if (isBraking)
    {
        const float LOW_SPEED_THRESHOLD = 0.1f;
        const float BRAKE_APPLIED_SPEED_THRESHOLD = 0.01f;
        const float BRAKING_FORCE = 4.f;
    
        if (fabs(speed) < LOW_SPEED_THRESHOLD)
            speed = 0.f;
            
        if (speed > BRAKE_APPLIED_SPEED_THRESHOLD)
            speed -= BRAKING_FORCE * deltaTime;
        else if (speed < -BRAKE_APPLIED_SPEED_THRESHOLD)
            speed += BRAKING_FORCE * deltaTime;
    }
    
    const float WHEELBASE = 2.7f;
    float angularSpeed = speed * sin(-glm::radians(steeringAngle)) / WHEELBASE;
    orientation.y += angularSpeed * deltaTime;
    
    glm::vec3 positionMod = glm::rotate(glm::mat4(1.0f), orientation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(-speed, 0.f, 0.f, 1.f);
    position += positionMod * deltaTime;
    
    const float WHEEL_RADIUS = 0.2f;
    const float PI = glm::pi<float>();
    wheelsRollAngle += speed / (2.f * PI * WHEEL_RADIUS) * deltaTime;
    
    if (wheelsRollAngle > PI)
        wheelsRollAngle -= 2.f * PI;
    else if (wheelsRollAngle < -PI)
        wheelsRollAngle += 2.f * PI;
        
    if (isRightBlinkerActivated || isLeftBlinkerActivated)
    {
        const float BLINKER_PERIOD_SEC = 0.5f;
        blinkerTimer += deltaTime;
        if (blinkerTimer > BLINKER_PERIOD_SEC)
        {
            blinkerTimer = 0.f;
            isBlinkerOn = !isBlinkerOn;
        }
    }
    else
    {
        isBlinkerOn = true;
        blinkerTimer = 0.f;
    }  
    carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, position);
    carModel = glm::rotate(carModel, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
}


void Car::draw(glm::mat4& projView, glm::mat4& view)
{
    drawFrame(projView, carModel, view);

    Material defaultCarMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 0.0f},
        {0.7f, 0.7f, 0.7f},
        10.0f
    };
    material->updateData(&defaultCarMat, 0, sizeof(Material));

    glActiveTexture(GL_TEXTURE0);
    if (carTexture)
        carTexture->use();

    drawWheels(projView, carModel, view);
}

void Car::drawWindows(glm::mat4& projView, glm::mat4& view)
{
    const glm::vec3 WINDOW_POSITION[] =
    {
        glm::vec3(-0.813, 0.755, 0.0),
        glm::vec3(1.092, 0.761, 0.0),
        glm::vec3(-0.3412, 0.757, 0.51),
        glm::vec3(-0.3412, 0.757, -0.51),
        glm::vec3(0.643, 0.756, 0.508),
        glm::vec3(0.643, 0.756, -0.508)
    };

    glm::mat4 carBase = glm::translate(glm::mat4(1.0f), position);
    carBase = glm::rotate(carBase, orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    carBase = glm::translate(carBase, glm::vec3(0.0f, 0.25f, 0.0f));

    std::map<float, unsigned int> sorted;
    for (unsigned int i = 0; i < 6; i++)
    {
        glm::vec4 viewPos = view * carBase * glm::vec4(WINDOW_POSITION[i], 1.0f);
        sorted[-viewPos.z] = i;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    celShadingShader->use();
    glActiveTexture(GL_TEXTURE0);
    carWindowTexture->use();

    for (std::map<float, unsigned int>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        glm::mat4 mvp = projView * carBase;
        celShadingShader->setMatrices(mvp, view, carBase);
        windows[it->second].draw();
    }

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

    
void Car::drawFrame(glm::mat4& projView, glm::mat4 carModel, glm::mat4& view)
{
    glm::mat4 frameModel = glm::translate(carModel, glm::vec3(0.0f, 0.25f, 0.0f));

    drawOutlinedModel(frame_, projView, view, frameModel);

    drawHeadlights(projView, frameModel, view);
}

void Car::drawWheel(glm::mat4& projView, glm::mat4 wheelModel, bool isFrontWheel, bool isLeftWheel, glm::mat4& view)
{
    const float WHEEL_ORIGIN_OFFSET = 0.10124f;
    float originOffset = isLeftWheel ? -WHEEL_ORIGIN_OFFSET : WHEEL_ORIGIN_OFFSET;

    if (isLeftWheel)
    {
        wheelModel = glm::rotate(wheelModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    wheelModel = glm::translate(wheelModel, glm::vec3(0.0f, 0.0f, originOffset));

    if (isFrontWheel)
    {
        wheelModel = glm::rotate(wheelModel, glm::radians(steeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    wheelModel = glm::rotate(wheelModel, wheelsRollAngle, glm::vec3(0.0f, 0.0f, 1.0f));
    wheelModel = glm::translate(wheelModel, glm::vec3(0.0f, 0.0f, -originOffset));

    drawOutlinedModel(wheel_, projView, view, wheelModel);
}

void Car::drawOutlinedModel(const Model& model, glm::mat4& projView, glm::mat4& view, glm::mat4 modelMatrix)
{
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    celShadingShader->use();
    glm::mat4 mvp = projView * modelMatrix;
    celShadingShader->setMatrices(mvp, view, modelMatrix);
    model.draw();

    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable(GL_CULL_FACE);
    edgeEffectShader->use();
    glUniformMatrix4fv(edgeEffectShader->mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
    model.draw();
    glEnable(GL_CULL_FACE);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);

    celShadingShader->use();
}

void Car::drawWheels(glm::mat4& projView, glm::mat4 carModel, glm::mat4& view)
{
    const glm::vec3 WHEEL_POSITIONS[] =
    {
        glm::vec3(-1.29f, 0.245f, -0.57f),  // Avant gauche
        glm::vec3(-1.29f, 0.245f,  0.57f),  // Avant droite
        glm::vec3( 1.4f , 0.245f, -0.57f),  // Arrière gauche
        glm::vec3( 1.4f , 0.245f,  0.57f)   // Arrière droite
    };
    
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 wheelModel = glm::translate(carModel, WHEEL_POSITIONS[i]);
        bool isFrontWheel = (i < 2);  
        bool isLeftWheel = (WHEEL_POSITIONS[i].z > 0.0f);
        drawWheel(projView, wheelModel, isFrontWheel, isLeftWheel, view);
    }
}

void Car::drawBlinker(glm::mat4& projView, glm::mat4 headlightModel, bool isLeftHeadlight, glm::mat4& view)
{
    glm::mat4 blinkerModel = glm::translate(headlightModel, glm::vec3(-0.05f, 0.0f, -0.06065f));

    bool isBlinkerActivated = (isLeftHeadlight && isLeftBlinkerActivated) ||
        (!isLeftHeadlight && isRightBlinkerActivated);

    const glm::vec3 ON_COLOR(1.0f, 0.7f, 0.3f);
    const glm::vec3 OFF_COLOR(0.5f, 0.35f, 0.15f);

    Material blinkerMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {OFF_COLOR, 0.0f},
        {OFF_COLOR, 0.0f},
        {OFF_COLOR},
        10.0f
    };

    if (isBlinkerOn && isBlinkerActivated)
        blinkerMat.emission = glm::vec4(ON_COLOR, 0.0f);

    material->updateData(&blinkerMat, 0, sizeof(Material));

    celShadingShader->use();
    glm::mat4 mvp = projView * blinkerModel;
    celShadingShader->setMatrices(mvp, view, blinkerModel);

    glActiveTexture(GL_TEXTURE0);
    if (lightTexture)
        lightTexture->use();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    blinker_.draw();

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}



void Car::drawLight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight, glm::mat4& view)
{
    glm::vec3 offset = isFrontHeadlight
        ? glm::vec3(-0.05f, 0.0f, 0.0f)
        : glm::vec3(0.05f, 0.0f, 0.0f);

    glm::mat4 lightModel = glm::translate(headlightModel, offset);

    const glm::vec3 FRONT_ON_COLOR(1.0f, 1.0f, 1.0f);
    const glm::vec3 FRONT_OFF_COLOR(0.5f, 0.5f, 0.5f);
    const glm::vec3 REAR_ON_COLOR(1.0f, 0.1f, 0.1f);
    const glm::vec3 REAR_OFF_COLOR(0.5f, 0.1f, 0.1f);

    Material lightMat =
    {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        10.0f
    };

    if (isFrontHeadlight)
    {
        lightMat.ambient = glm::vec4(FRONT_OFF_COLOR, 0.0f);
        lightMat.diffuse = glm::vec4(FRONT_OFF_COLOR, 0.0f);
        lightMat.specular = FRONT_OFF_COLOR;
        if (isHeadlightOn)
            lightMat.emission = glm::vec4(FRONT_ON_COLOR, 0.0f);
    }
    else
    {
        lightMat.ambient = glm::vec4(REAR_OFF_COLOR, 0.0f);
        lightMat.diffuse = glm::vec4(REAR_OFF_COLOR, 0.0f);
        lightMat.specular = REAR_OFF_COLOR;
        if (isBraking)
            lightMat.emission = glm::vec4(REAR_ON_COLOR, 0.0f);
    }

    material->updateData(&lightMat, 0, sizeof(Material));

    celShadingShader->use();
    glm::mat4 mvp = projView * lightModel;
    celShadingShader->setMatrices(mvp, view, lightModel);

    glActiveTexture(GL_TEXTURE0);
    if (lightTexture)
        lightTexture->use();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    light_.draw();

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Car::drawHeadlight(glm::mat4& projView, glm::mat4 headlightModel, bool isFrontHeadlight, bool isLeftHeadlight, glm::mat4& view)
{
 
    if (isFrontHeadlight)
    {
        headlightModel = glm::rotate(headlightModel, glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    drawLight(projView, headlightModel, isFrontHeadlight, view);
    if (isFrontHeadlight)
    {
        drawBlinker(projView, headlightModel, isLeftHeadlight, view);
    }
}

void Car::drawHeadlights(glm::mat4& projView, glm::mat4 frameModel, glm::mat4& view)
{
    const glm::vec3 HEADLIGHT_POSITIONS[] =
    {
        glm::vec3(-1.9650f, 0.38f, -0.45f),  // Avant gauche
        glm::vec3(-1.9650f, 0.38f,  0.45f),  // Avant droite
        glm::vec3( 2.0019f, 0.38f, -0.45f),  // Arrière gauche
        glm::vec3( 2.0019f, 0.38f,  0.45f)   // Arrière droite
    };
    
    for (int i = 0; i < 4; ++i)
    {
        glm::mat4 headlightModel = glm::translate(frameModel, HEADLIGHT_POSITIONS[i]);
        bool isFrontHeadlight = (i < 2);      
        bool isLeftHeadlight = (i % 2 == 0);  
        
        drawHeadlight(projView, headlightModel, isFrontHeadlight, isLeftHeadlight, view);
    }
}


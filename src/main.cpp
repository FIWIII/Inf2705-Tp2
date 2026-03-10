#include <cstddef>
#include <cstdint>
#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "happly.h"
#include <imgui/imgui.h>
#include <inf2705/OpenGLApplication.hpp>
#include "model.hpp"
#include "car.hpp"
#include "model_data.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "uniform_buffer.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;


struct Material
{
    glm::vec4 emission; 
    glm::vec4 ambient;  
    glm::vec4 diffuse; 
    glm::vec3 specular;
    GLfloat shininess;
};

struct DirectionalLight
{
    glm::vec4 ambient;  
    glm::vec4 diffuse;   
    glm::vec4 specular;  
    glm::vec4 direction; 
};

struct SpotLight
{
    glm::vec4 ambient;   
    glm::vec4 diffuse;   
    glm::vec4 specular;  

    glm::vec4 position;
    glm::vec3 direction;
    GLfloat exponent;
    GLfloat openingAngle;

    GLfloat padding[3];
};

// Matériels

Material defaultMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material grassMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.05f, 0.05f, 0.05f},
    100.0f
};

Material streetMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.7f, 0.7f, 0.7f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.025f, 0.025f, 0.025f},
    300.0f
};

Material streetlightMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.8f, 0.8f, 0.8f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material streetlightLightMat =
{
    {0.8f, 0.7f, 0.5f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {0.7f, 0.7f, 0.7f},
    10.0f
};

Material windowMat =
{
    {0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 1.0f},
    2.0f
};


struct App : public OpenGLApplication
{
    App()
    : isDay_(true)
    , cameraPosition_(0.f, 0.f, 0.f)
    , cameraOrientation_(0.f, 0.f)
    , isMouseMotionEnabled_(false)
    , isAutopilotEnabled_(true)
    , trackDistance_(0.0f)
    , currentScene_(0)
    {
        car_.position = glm::vec3(0.0f, 0.0f, 15.0f);
        car_.orientation.y = glm::radians(180.0f);
        car_.speed = 4.0f;
    }
	
	void init() override
	{
		
		setKeybindMessage(
			"ESC : quitter l'application." "\n"
			"T : changer de scène." "\n"
			"W : déplacer la caméra vers l'avant." "\n"
			"S : déplacer la caméra vers l'arrière." "\n"
			"A : déplacer la caméra vers la gauche." "\n"
			"D : déplacer la caméra vers la droite." "\n"
			"Q : déplacer la caméra vers le bas." "\n"
			"E : déplacer la caméra vers le haut." "\n"
			"Flèches : tourner la caméra." "\n"
			"Souris : tourner la caméra" "\n"
			"Espace : activer/désactiver la souris." "\n"
		);


		glClearColor(0.5f, 0.5f, 0.5f, 1.0f); 
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        
        edgeEffectShader_.create();
        celShadingShader_.create();
        skyShader_.create();

        car_.edgeEffectShader = &edgeEffectShader_;
        car_.celShadingShader = &celShadingShader_;
        car_.material = &material_;
        car_.carTexture = &carTexture_;
        car_.carWindowTexture = &carWindowTexture_;
        car_.lightTexture = &streetlightLightTexture_;


 
        grassTexture_.load("../textures/grass.jpg");
        streetTexture_.load("../textures/street.jpg");
        streetcornerTexture_.load("../textures/streetcorner.jpg");
        carTexture_.load("../textures/car.png");
        carWindowTexture_.load("../textures/window.png");
        treeTexture_.load("../textures/pine.jpg");
        streetlightTexture_.load("../textures/streetlight.jpg");
        streetlightLightTexture_.load("../textures/light.png");
        

        const char* pathes[] = {
            "../textures/skybox/Daylight Box_Right.bmp",
            "../textures/skybox/Daylight Box_Left.bmp",
            "../textures/skybox/Daylight Box_Top.bmp",
            "../textures/skybox/Daylight Box_Bottom.bmp",
            "../textures/skybox/Daylight Box_Front.bmp",
            "../textures/skybox/Daylight Box_Back.bmp",
        };

        const char* nightPathes[] = {
            "../textures/skyboxNight/right.png",
            "../textures/skyboxNight/left.png",
            "../textures/skyboxNight/top.png",
            "../textures/skyboxNight/bottom.png",
            "../textures/skyboxNight/front.png",
            "../textures/skyboxNight/back.png",
        };

        skyboxTexture_.load(pathes);
        skyboxNightTexture_.load(nightPathes);
        loadModels();
        initStaticMatrices();

        grassTexture_.setWrap(GL_REPEAT);
        grassTexture_.setFiltering(GL_LINEAR);
        grassTexture_.enableMipmap();

        streetTexture_.setWrap(GL_REPEAT);
        streetTexture_.setFiltering(GL_LINEAR);
        streetTexture_.enableMipmap();

        streetcornerTexture_.setWrap(GL_CLAMP_TO_EDGE);
        streetcornerTexture_.setFiltering(GL_LINEAR);

        carTexture_.setWrap(GL_CLAMP_TO_EDGE);
        carTexture_.setFiltering(GL_LINEAR);

        carWindowTexture_.setWrap(GL_CLAMP_TO_EDGE);
        carWindowTexture_.setFiltering(GL_NEAREST);

        treeTexture_.setWrap(GL_REPEAT);
        treeTexture_.setFiltering(GL_NEAREST);

        streetlightTexture_.setWrap(GL_REPEAT);
        streetlightTexture_.setFiltering(GL_LINEAR);

        streetlightLightTexture_.setWrap(GL_CLAMP_TO_EDGE);
        streetlightLightTexture_.setFiltering(GL_NEAREST);

        // Partie 3
        material_.allocate(&defaultMat, sizeof(Material));
        material_.setBindingIndex(0);

        lightsData_.dirLight =
        {
            {0.2f, 0.2f, 0.2f, 0.0f},
            {1.0f, 1.0f, 1.0f, 0.0f},
            {0.5f, 0.5f, 0.5f, 0.0f},
            {0.5f, -1.0f, 0.5f, 0.0f}
        };

        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            lightsData_.spotLights[i].position = glm::vec4(streetlightLightPositions[i], 1.0f);
            lightsData_.spotLights[i].direction = glm::vec3(0, -1, 0);
            lightsData_.spotLights[i].exponent = 6.0f;
            lightsData_.spotLights[i].openingAngle = 60.f;
        }

        // Intialisation basique des spots de la voiture (phares avant)
        lightsData_.spotLights[N_STREETLIGHTS].position = glm::vec4(-1.6, 0.64, -0.45, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS].openingAngle = 30.f;

        lightsData_.spotLights[N_STREETLIGHTS + 1].position = glm::vec4(-1.6, 0.64, 0.45, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS + 1].direction = glm::vec3(-10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS + 1].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS + 1].openingAngle = 30.f;

        // Feux arrière (freins)
        lightsData_.spotLights[N_STREETLIGHTS + 2].position = glm::vec4(1.6, 0.64, -0.45, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS + 2].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS + 2].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS + 2].openingAngle = 30.f;

        lightsData_.spotLights[N_STREETLIGHTS + 3].position = glm::vec4(1.6, 0.64, 0.45, 1.0f);
        lightsData_.spotLights[N_STREETLIGHTS + 3].direction = glm::vec3(10, -1, 0);
        lightsData_.spotLights[N_STREETLIGHTS + 3].exponent = 4.0f;
        lightsData_.spotLights[N_STREETLIGHTS + 3].openingAngle = 30.f;

        toggleStreetlight();
        updateCarLight();
        setLightingUniform();

        lights_.allocate(&lightsData_, sizeof(lightsData_));
        lights_.setBindingIndex(1);

        CHECK_GL_ERROR;
        
	}
	
	

	void drawFrame() override
	{
        CHECK_GL_ERROR;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);

        if (ImGui::Button("Reload Shaders"))
        {
            CHECK_GL_ERROR;
            edgeEffectShader_.reload();
            celShadingShader_.reload();
            skyShader_.reload();

            setLightingUniform();
            CHECK_GL_ERROR;
        }
        ImGui::End();

        switch (currentScene_)
        {
        case 0: sceneMain(); break;
        }
        CHECK_GL_ERROR;
	}

	void onKeyPress(const sf::Event::KeyPressed& key) override
	{
		using enum sf::Keyboard::Key;
		switch (key.code)
		{
		    case Escape:
		        window_.close();
	        break;
		    case Space:
		        isMouseMotionEnabled_ = !isMouseMotionEnabled_;
		        if (isMouseMotionEnabled_)
		        {
		            window_.setMouseCursorGrabbed(true);
		            window_.setMouseCursorVisible(false);
	            }
	            else
	            {
	                window_.setMouseCursorGrabbed(false);
	                window_.setMouseCursorVisible(true);
                }
	        break;
	        case T:
            break;
		    default: break;
		}
	}

    void onResize(const sf::Event::Resized& event) override
    {
    }

	void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
	{	    
	    if (!isMouseMotionEnabled_)
	        return;
        
        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
	    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
	}
	
	void updateCameraInput() 
    {
        if (!window_.hasFocus())
            return;
            
        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }
        
        float cameraMouvementX = 0;
        float cameraMouvementY = 0;
        
        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;
        
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;


        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;
            
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }
    
    void loadModels()
    {
        car_.loadModels();
        tree_.load("../models/pine.ply");
        streetlight_.load("../models/streetlight.ply");
        streetlightLight_.load("../models/streetlight_light.ply");
        skybox_.load("../models/skybox.ply");

        grass_.load(ground, sizeof(ground), planeElements, sizeof(planeElements));
        street_.load(street, sizeof(street), planeElements, sizeof(planeElements));
        streetcorner_.load(streetcorner, sizeof(streetcorner), planeElements, sizeof(planeElements));
      
    }
    
    
    void drawModel(const Model& model, glm::mat4& projView, glm::mat4& view, glm::mat4 modelMatrix)
    {
        celShadingShader_.use();   
        glm::mat4 mvp = projView * modelMatrix;
        celShadingShader_.setMatrices(mvp, view, modelMatrix); 
        model.draw();
    }

    void drawOutlinedModel(const Model& model, glm::mat4& projView, glm::mat4& view, glm::mat4 modelMatrix)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        celShadingShader_.use();
        glm::mat4 mvp = projView * modelMatrix;
        celShadingShader_.setMatrices(mvp, view, modelMatrix);
        model.draw();

        glStencilMask(0x00);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glDisable(GL_CULL_FACE);
        edgeEffectShader_.use();
        glUniformMatrix4fv(edgeEffectShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));
        model.draw();
        glEnable(GL_CULL_FACE);

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glDisable(GL_STENCIL_TEST);
    }

    void drawStreetlights(glm::mat4& projView, glm::mat4& view)
    {
      
        for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
        {
            if (!isDay_) setMaterial(streetlightLightMat);
            else setMaterial(streetlightMat);

            streetlightLightTexture_.use();
            drawOutlinedModel(streetlightLight_, projView, view, streetlightModelMatrices_[i]);

            setMaterial(streetlightMat);
            streetlightTexture_.use();
            drawOutlinedModel(streetlight_, projView, view, streetlightModelMatrices_[i]);
        }
    }

    void drawTree(glm::mat4& projView, glm::mat4& view)
    {
        glDisable(GL_CULL_FACE);
        for (unsigned int i = 0; i < N_TREES; i++) {
            treeTexture_.use();
            drawOutlinedModel(tree_, projView, view, treeModelMatrices_[i]);
        }
        glEnable(GL_CULL_FACE);
    }

    
    void drawGround(glm::mat4& projView, glm::mat4& view) 
    {

        setMaterial(streetMat);
        streetcornerTexture_.use();
        for (int i = 0; i < 4; ++i) drawModel(streetcorner_, projView, view, streetPatchesModelMatrices_[i]);
		streetTexture_.use();
        for (int i = 4; i < N_STREET_PATCHES; ++i) drawModel(street_, projView, view, streetPatchesModelMatrices_[i]);

        setMaterial(grassMat);
        grassTexture_.use();
        drawModel(grass_, projView, view, groundModelMatrice_);
    }

    glm::mat4 getViewMatrix()
    {
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, -cameraOrientation_.x, glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, -cameraOrientation_.y, glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, -cameraPosition_);
        return view;
    }

    glm::mat4 getPerspectiveProjectionMatrix()      
    {       
        
        float fov = glm::radians(70.0f);  
        float aspect = getWindowAspect();  
        float nearPlane = 0.1f;
        float farPlane = 300.0f;
        return glm::perspective(fov, aspect, nearPlane, farPlane);
    }

    
    void setLightingUniform()
    {
        celShadingShader_.use();
        glUniform1i(celShadingShader_.nSpotLightsULoc, N_STREETLIGHTS + 4);

        float ambientIntensity = 0.05;
        glUniform3f(celShadingShader_.globalAmbientULoc, ambientIntensity, ambientIntensity, ambientIntensity);
    }

    
    void toggleSun()
    {
        if (isDay_)
        {
            lightsData_.dirLight.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.5f, 0.5f, 0.5f, 0.0f);
        }
        else
        {
            lightsData_.dirLight.ambient = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
            lightsData_.dirLight.specular = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    void toggleStreetlight()
    {
        if (isDay_)
        {
            for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.0f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.0f), 0.0f);
            }
        }
        else
        {
            for (unsigned int i = 0; i < N_STREETLIGHTS; i++)
            {
                lightsData_.spotLights[i].ambient = glm::vec4(glm::vec3(0.02f), 0.0f);
                lightsData_.spotLights[i].diffuse = glm::vec4(glm::vec3(0.8f), 0.0f);
                lightsData_.spotLights[i].specular = glm::vec4(glm::vec3(0.4f), 0.0f);
            }
        }
    }

    void updateCarLight()
    {
        if (car_.isHeadlightOn)
        {
            lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(glm::vec3(0.01), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(glm::vec3(1.0), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(glm::vec3(0.4), 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 1].ambient = glm::vec4(glm::vec3(0.01), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].diffuse = glm::vec4(glm::vec3(1.0), 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].specular = glm::vec4(glm::vec3(0.4), 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS].position = car_.carModel * glm::vec4(-1.6, 0.64, -0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS].direction = glm::mat3(car_.carModel) * glm::vec3(-10, -1, 0);

            lightsData_.spotLights[N_STREETLIGHTS + 1].position = car_.carModel * glm::vec4(-1.6, 0.64, 0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].direction = glm::mat3(car_.carModel) * glm::vec3(-10, -1, 0);
        }
        else
        {
            lightsData_.spotLights[N_STREETLIGHTS].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS].specular = glm::vec4(0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 1].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 1].specular = glm::vec4(0.0f);
        }

        if (car_.isBraking)
        {
            lightsData_.spotLights[N_STREETLIGHTS + 2].ambient = glm::vec4(0.01, 0.0, 0.0, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].diffuse = glm::vec4(0.9, 0.1, 0.1, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].specular = glm::vec4(0.35, 0.05, 0.05, 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 3].ambient = glm::vec4(0.01, 0.0, 0.0, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].diffuse = glm::vec4(0.9, 0.1, 0.1, 0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].specular = glm::vec4(0.35, 0.05, 0.05, 0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 2].position = car_.carModel * glm::vec4(1.6, 0.64, -0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].direction = glm::mat3(car_.carModel) * glm::vec3(10, -1, 0);

            lightsData_.spotLights[N_STREETLIGHTS + 3].position = car_.carModel * glm::vec4(1.6, 0.64, 0.45, 1.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].direction = glm::mat3(car_.carModel) * glm::vec3(10, -1, 0);
        }
        else
        {
            lightsData_.spotLights[N_STREETLIGHTS + 2].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 2].specular = glm::vec4(0.0f);

            lightsData_.spotLights[N_STREETLIGHTS + 3].ambient = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].diffuse = glm::vec4(0.0f);
            lightsData_.spotLights[N_STREETLIGHTS + 3].specular = glm::vec4(0.0f);
        }
    }
    void drawSkybox(glm::mat4& view, glm::mat4& proj)
    {
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);

        skyShader_.use();

        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
        glm::mat4 mvp = proj * viewNoTranslation;
        glUniformMatrix4fv(skyShader_.mvpULoc, 1, GL_FALSE, glm::value_ptr(mvp));

        glActiveTexture(GL_TEXTURE0);
        if (isDay_)
            skyboxTexture_.use();
        else
            skyboxNightTexture_.use();

        skybox_.draw();

        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
    }

    void setMaterial(Material& mat)
    {
        material_.updateData(&mat, 0, sizeof(Material));
    }
   
    void sceneMain()
    {
        ImGui::Begin("Scene Parameters");
        if (ImGui::Button("Toggle Day/Night"))
        {
            isDay_ = !isDay_;
            toggleSun();
            toggleStreetlight();
            lights_.updateData(&lightsData_, 0, sizeof(DirectionalLight) + N_STREETLIGHTS * sizeof(SpotLight));
        }
        ImGui::SliderFloat("Car Speed", &car_.speed, -10.0f, 10.0f, "%.2f m/s");
        ImGui::SliderFloat("Steering Angle", &car_.steeringAngle, -30.0f, 30.0f, "%.2f°");
        if (ImGui::Button("Reset Steering"))
            car_.steeringAngle = 0.f;
        ImGui::Checkbox("Headlight", &car_.isHeadlightOn);
        ImGui::Checkbox("Left Blinker", &car_.isLeftBlinkerActivated);
        ImGui::Checkbox("Right Blinker", &car_.isRightBlinkerActivated);
        ImGui::Checkbox("Brake", &car_.isBraking);
        ImGui::Checkbox("Auto drive", &isAutopilotEnabled_);
        ImGui::End();

        updateCameraInput();
        if (isAutopilotEnabled_) {
            updateCarOnTrack(deltaTime_);
        }

        car_.update(deltaTime_);

        updateCarLight();

        lights_.updateData(&lightsData_.spotLights[N_STREETLIGHTS], sizeof(DirectionalLight) + N_STREETLIGHTS * sizeof(SpotLight), 4 * sizeof(SpotLight));

        glm::mat4 view = getViewMatrix();
        glm::mat4 proj = getPerspectiveProjectionMatrix();
        glm::mat4 projView = proj * view;

        drawSkybox(view, proj);

        celShadingShader_.use();

        glActiveTexture(GL_TEXTURE0);

        setMaterial(streetMat);
        drawGround(projView, view);

        setMaterial(defaultMat);
        drawTree(projView, view);

        setMaterial(streetlightMat);
        drawStreetlights(projView, view);

        setMaterial(defaultMat);
        carTexture_.use();
        car_.draw(projView, view);

        setMaterial(windowMat);
        carWindowTexture_.use();
        car_.drawWindows(projView, view);

        glDisable(GL_BLEND);

    }

    void initStaticMatrices()
    {
        const float ROAD_HALF_LENGTH = 15.0f;
        const float ROAD_WIDTH = 5.0f;
        const float SEGMENT_LENGTH = 30.0f / 7.0f;

        groundModelMatrice_ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f));
        groundModelMatrice_ = glm::scale(groundModelMatrice_, glm::vec3(50.0f, 1.0f, 50.0f));
        treeModelMatrices_[0] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        treeModelMatrices_[0] = glm::scale(treeModelMatrices_[0], glm::vec3(15.0f, 15.0f, 15.0f));

        const float lightOffset = ROAD_HALF_LENGTH - (ROAD_WIDTH * 0.5f) - 0.25f;
        const glm::vec3 positions[] =
        {
            { -7.5f, -0.15f, -lightOffset },
            {  7.5f, -0.15f, -lightOffset },
            { -7.5f, -0.15f,  lightOffset },
            {  7.5f, -0.15f,  lightOffset },
            { -lightOffset, -0.15f, -7.5f },
            { -lightOffset, -0.15f,  7.5f },
            {  lightOffset, -0.15f, -7.5f },
            {  lightOffset, -0.15f,  7.5f }
        };

        for (int i = 0; i < N_STREETLIGHTS; ++i)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), positions[i]);
            glm::vec3 toCenter = glm::normalize(glm::vec3(0.0f) - positions[i]);
            float rotation = std::atan2(-toCenter.x, -toCenter.z) + glm::radians(90.0f);
            streetlightModelMatrices_[i] = glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
            streetlightLightPositions[i] = glm::vec3(streetlightModelMatrices_[i] * glm::vec4(-2.77, 5.2, 0.0, 1.0));
        }

        int patchIndex = 0;
        const glm::vec3 corners[] =
        {
            { -ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH },
            {  ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH },
            { -ROAD_HALF_LENGTH, 0.0f,  ROAD_HALF_LENGTH },
            {  ROAD_HALF_LENGTH, 0.0f,  ROAD_HALF_LENGTH }
        };

        for (const auto& pos : corners)
        {
            glm::mat4 cornerModel = glm::translate(glm::mat4(1.0f), pos);
            streetPatchesModelMatrices_[patchIndex++] = glm::scale(cornerModel, glm::vec3(ROAD_WIDTH, 1.0f, ROAD_WIDTH));
        }

        for (int i = 0; i < 7; ++i)
        {
            float x = -ROAD_HALF_LENGTH + (SEGMENT_LENGTH * 0.5f) + i * SEGMENT_LENGTH;

            glm::mat4 top = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, -ROAD_HALF_LENGTH));
            streetPatchesModelMatrices_[patchIndex++] = glm::scale(top, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));

            glm::mat4 bottom = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, ROAD_HALF_LENGTH));
            streetPatchesModelMatrices_[patchIndex++] = glm::scale(bottom, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));
        }

        for (int i = 0; i < 7; ++i)
        {
            float z = -ROAD_HALF_LENGTH + (SEGMENT_LENGTH * 0.5f) + i * SEGMENT_LENGTH;

            glm::mat4 left = glm::translate(glm::mat4(1.0f), glm::vec3(-ROAD_HALF_LENGTH, 0.0f, z));
            left = glm::rotate(left, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            streetPatchesModelMatrices_[patchIndex++] = glm::scale(left, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));

            glm::mat4 right = glm::translate(glm::mat4(1.0f), glm::vec3(ROAD_HALF_LENGTH, 0.0f, z));
            right = glm::rotate(right, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            streetPatchesModelMatrices_[patchIndex++] = glm::scale(right, glm::vec3(SEGMENT_LENGTH, 1.0f, ROAD_WIDTH));
        }
    }
    
    void updateCarOnTrack(float deltaTime)
    {
        const float ROAD_HALF_LENGTH = 15.0f;
        const float SEGMENT_LENGTH = ROAD_HALF_LENGTH * 2.0f;
        const float PERIMETER = 4.0f * SEGMENT_LENGTH;

        trackDistance_ += car_.speed * deltaTime;
        trackDistance_ = std::fmod(trackDistance_, PERIMETER);
        if (trackDistance_ < 0.0f)
            trackDistance_ += PERIMETER;

        float d = trackDistance_;

        if (d < SEGMENT_LENGTH)
        {
            car_.position = glm::vec3(-ROAD_HALF_LENGTH + d, 0.0f, ROAD_HALF_LENGTH);
            car_.orientation.y = glm::radians(180.0f);
        }
        else if (d < 2.0f * SEGMENT_LENGTH)
        {
            d -= SEGMENT_LENGTH;
            car_.position = glm::vec3(ROAD_HALF_LENGTH, 0.0f, ROAD_HALF_LENGTH - d);
            car_.orientation.y = glm::radians(-90.0f);
        }
        else if (d < 3.0f * SEGMENT_LENGTH)
        {
            d -= 2.0f * SEGMENT_LENGTH;
            car_.position = glm::vec3(ROAD_HALF_LENGTH - d, 0.0f, -ROAD_HALF_LENGTH);
            car_.orientation.y = 0.0f;
        }
        else
        {
            d -= 3.0f * SEGMENT_LENGTH;
            car_.position = glm::vec3(-ROAD_HALF_LENGTH, 0.0f, -ROAD_HALF_LENGTH + d);
            car_.orientation.y = glm::radians(90.0f);
        }
    }
    
private:
    // Shaders
    EdgeEffect edgeEffectShader_;
    CelShading celShadingShader_;
    Sky skyShader_;

    // Textures
    Texture2D grassTexture_;
    Texture2D streetTexture_;
    Texture2D streetcornerTexture_;
    Texture2D carTexture_;
    Texture2D carWindowTexture_;
    Texture2D treeTexture_;
    Texture2D streetlightTexture_;
    Texture2D streetlightLightTexture_;
    TextureCubeMap skyboxTexture_;
    TextureCubeMap skyboxNightTexture_;

    struct {
        DirectionalLight dirLight;
        SpotLight spotLights[12];
    } lightsData_;


    // Uniform buffers
    UniformBuffer material_;
    UniformBuffer lights_;

    Model tree_;
    Model streetlight_;
    Model streetlightLight_;
    Model grass_;
    Model street_;
    Model streetcorner_;
    Model skybox_;
    
    Car car_;
    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;
    
    // Matrices
    static constexpr unsigned int N_TREES = 1;
    static constexpr unsigned int N_STREETLIGHTS = 8;
    static constexpr unsigned int N_STREET_PATCHES = 7 * 4 + 4;
    glm::mat4 treeModelMatrices_[N_TREES];
    glm::mat4 streetlightModelMatrices_[N_STREETLIGHTS];
    glm::mat4 streetPatchesModelMatrices_[N_STREET_PATCHES];
    glm::mat4 groundModelMatrice_;
    glm::vec3 streetlightLightPositions[N_STREETLIGHTS];
    
    bool isDay_;
    bool isMouseMotionEnabled_;
    bool isAutopilotEnabled_;
    float trackDistance_;

    // Imgui var
    const char* const SCENE_NAMES[1] = {
        "Main scene"
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;

};


int main(int argc, char* argv[])
{
	WindowSettings settings = {};
	settings.fps = 60;
	settings.context.depthBits = 24;
	settings.context.stencilBits = 8;
	settings.context.antiAliasingLevel = 4;
	settings.context.majorVersion = 3;
	settings.context.minorVersion = 3;
	settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

	App app;
	app.run(argc, argv, "Tp2", settings);
}

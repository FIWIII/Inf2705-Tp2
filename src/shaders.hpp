#include "shader_program.hpp"

#include <glm/glm.hpp>

class TransformShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint colorModULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};



class EdgeEffect : public ShaderProgram
{
public:
    GLuint mvpULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class Sky : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint textureSamplerULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class CelShading : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint viewULoc;
    GLuint modelViewULoc;
    GLuint normalULoc;
    
    GLuint nSpotLightsULoc;
    
    GLuint globalAmbientULoc;
    GLuint diffuseSamplerULoc;

public:
    void setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
    virtual void assignAllUniformBlockIndexes() override;
};


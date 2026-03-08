#include "shader_program.hpp"

#include <glm/glm.hpp>

// Implémentation de vos shaders ici.
// Ils doivent hérité de ShaderProgram et implémenter les méthodes virtuelles pures
// load() et getAllUniformLocations().
// Mémoriser les uniforms locations dans des attributs public. Vous pouvez ajouter ce que
// vous voulez dans les classes et mieux séparer le code.
// Voir exemple avec le shader du tp1, considérant les variables uniformes dans le shader:

// Exemple, à retirer plus tard
class TransformShader : public ShaderProgram
{
public:
    GLuint mvpULoc;
    GLuint colorModULoc;

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


// TODO: Compléter ou ajouter ici bas:

class EdgeEffect : public ShaderProgram
{
public:
    // TODO: Uniform location ou autre

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
};


class Sky : public ShaderProgram
{
public:
    // TODO: Uniform location ou autre

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

public:
    void setMatrices(glm::mat4& mvp, glm::mat4& view, glm::mat4& model);

protected:
    virtual void load() override;
    virtual void getAllUniformLocations() override;
    virtual void assignAllUniformBlockIndexes() override;
};


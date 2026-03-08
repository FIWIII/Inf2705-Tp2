#include "model.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstddef>

#include "happly.h"

using namespace gl;

struct PositionAttribute
{
    float x, y, z;
};

struct ColorUCharAttribute
{
    unsigned char r, g, b;
};

struct NormalAttribute
{
    float x, y, z;
};

struct TexCoordAttribute
{
    float s, t;
};

struct VertexModel
{
    PositionAttribute pos;
    ColorUCharAttribute color;
    NormalAttribute normal;
    TexCoordAttribute texCoord;
};

const GLuint VERTEX_POSITION_INDEX = 0;
const GLuint VERTEX_COLOR_INDEX = 1;
const GLuint VERTEX_NORMAL_INDEX = 2;
const GLuint VERTEX_TEXCOORDS_INDEX = 3;

void Model::load(const char* path)
{
    happly::PLYData plyIn(path);

    happly::Element& vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");

    std::vector<float> normalX, normalY, normalZ;
    try
    {
        normalX = vertex.getProperty<float>("nx");
        normalY = vertex.getProperty<float>("ny");
        normalZ = vertex.getProperty<float>("nz");
    }
    catch (std::runtime_error&)
    {
        std::cout << "No normal attribute for model \"" << path << "\"" << std::endl;
    }

    std::vector<unsigned char> colorRed, colorGreen, colorBlue;
    try
    {
        colorRed = vertex.getProperty<unsigned char>("red");
        colorGreen = vertex.getProperty<unsigned char>("green");
        colorBlue = vertex.getProperty<unsigned char>("blue");
    }
    catch (std::runtime_error&)
    {
        std::cout << "No color attribute for model \"" << path << "\"" << std::endl;
    }

    std::vector<float> texCoordsX, texCoordsY;
    try
    {
        texCoordsX = vertex.getProperty<float>("s");
        texCoordsY = vertex.getProperty<float>("t");
    }
    catch (std::runtime_error&)
    {
        std::cout << "No texture coordinate attribute for model \"" << path << "\"" << std::endl;
    }

    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();

    std::vector<VertexModel> vPos(positionX.size());
    for (size_t i = 0; i < vPos.size(); i++)
    {
        vPos[i] = {};

        vPos[i].pos.x = positionX[i];
        vPos[i].pos.y = positionY[i];
        vPos[i].pos.z = positionZ[i];

        if (!colorRed.empty())
        {
            vPos[i].color.r = colorRed[i];
            vPos[i].color.g = colorGreen[i];
            vPos[i].color.b = colorBlue[i];
        }

        if (!normalX.empty())
        {
            vPos[i].normal.x = normalX[i];
            vPos[i].normal.y = normalY[i];
            vPos[i].normal.z = normalZ[i];
        }

        if (!texCoordsX.empty())
        {
            vPos[i].texCoord.s = texCoordsX[i];
            vPos[i].texCoord.t = texCoordsY[i];
        }
    }

    std::vector<unsigned int> elementsData(facesIndices.size() * 3);
    for (size_t i = 0; i < facesIndices.size(); i++)
    {
        for (size_t j = 0; j < facesIndices[i].size(); j++)
        {
            elementsData[3 * i + j] = facesIndices[i][j];
        }
    }

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(VertexModel), vPos.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsData.size() * sizeof(unsigned int), elementsData.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glEnableVertexAttribArray(VERTEX_POSITION_INDEX);
    glVertexAttribPointer(VERTEX_POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, pos)));

    if (!colorRed.empty())
    {
        glEnableVertexAttribArray(VERTEX_COLOR_INDEX);
        glVertexAttribPointer(VERTEX_COLOR_INDEX, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, color)));
    }
    else
    {
        glDisableVertexAttribArray(VERTEX_COLOR_INDEX);
    }

    if (!normalX.empty())
    {
        glEnableVertexAttribArray(VERTEX_NORMAL_INDEX);
        glVertexAttribPointer(VERTEX_NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, normal)));
    }
    else
    {
        glDisableVertexAttribArray(VERTEX_NORMAL_INDEX);
    }

    if (!texCoordsX.empty())
    {
        glEnableVertexAttribArray(VERTEX_TEXCOORDS_INDEX);
        glVertexAttribPointer(VERTEX_TEXCOORDS_INDEX, 2, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (GLvoid*)(offsetof(VertexModel, texCoord)));
    }
    else
    {
        glDisableVertexAttribArray(VERTEX_TEXCOORDS_INDEX);
    }

    glBindVertexArray(0);

    count_ = static_cast<GLsizei>(elementsData.size());
}

void Model::load(float* vertices, size_t verticesSize, unsigned int* elements, size_t elementsSize)
{
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementsSize, elements, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glEnableVertexAttribArray(VERTEX_POSITION_INDEX);
    glVertexAttribPointer(VERTEX_POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(0));

    glDisableVertexAttribArray(VERTEX_COLOR_INDEX);
    glDisableVertexAttribArray(VERTEX_NORMAL_INDEX);

    glEnableVertexAttribArray(VERTEX_TEXCOORDS_INDEX);
    glVertexAttribPointer(VERTEX_TEXCOORDS_INDEX, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

    glBindVertexArray(0);

    count_ = static_cast<GLsizei>(elementsSize / sizeof(unsigned int));
}

Model::~Model()
{
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
}

void Model::draw() const
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
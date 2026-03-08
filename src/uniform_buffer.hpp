#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include <glbinding/gl/gl.h>

using namespace gl;

class UniformBuffer
{
public:
    UniformBuffer();
    ~UniformBuffer();
    
    void allocate(const void* data, GLsizeiptr byteSize);
    
    void setBindingIndex(GLuint index);

    void updateData(const void* data, GLintptr offset, GLsizeiptr byteSize);
    
private:
    GLuint id_;
};

#endif // UNIFORM_BUFFER_H

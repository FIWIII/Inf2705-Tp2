#include "uniform_buffer.hpp"

UniformBuffer::UniformBuffer()
{
}

UniformBuffer::~UniformBuffer()
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glDeleteBuffers(1, &id_);
}

void UniformBuffer::allocate(const void* data, GLsizeiptr byteSize)
{
    glGenBuffers(1, &id_);
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
    glBufferData(GL_UNIFORM_BUFFER, byteSize, data, GL_DYNAMIC_DRAW);
}

void UniformBuffer::setBindingIndex(GLuint index)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, id_);
}

void UniformBuffer::updateData(const void* data, GLintptr offset, GLsizeiptr byteSize)
{
    glBindBuffer(GL_UNIFORM_BUFFER, id_);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, byteSize, data);
}

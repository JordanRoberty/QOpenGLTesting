#ifndef ISHADER_MODULE_H
#define ISHADER_MODULE_H

#include <string>
#include <QOpenGLFunctions>

class IShaderModule
{
  public:
    virtual std::string getUniformDeclarations() const = 0;
    virtual std::string getFunctionImplementation() const = 0;
    virtual std::string getFunctionCall() const = 0;
    virtual void setupUniforms(uint shaderProgram) const = 0;
    virtual ~IShaderModule() = default;
};

#endif // ISHADER_MODULE_H
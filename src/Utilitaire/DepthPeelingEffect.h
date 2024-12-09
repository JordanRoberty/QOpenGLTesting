#ifndef DEPTH_PEELING_EFFECT_H
#define DEPTH_PEELING_EFFECT_H

#include "IShaderModule.h"

class DepthPeelingEffect : public IShaderModule
{
  public:
    DepthPeelingEffect();
    std::string getUniformDeclarations() const override;
    std::string getFunctionImplementation() const override;
    std::string getFunctionCall() const override;
    void setupUniforms(uint shaderProgram) const override;
};

#endif // DEPTH_PEELING_EFFECT_H
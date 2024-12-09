#ifndef BLINN_PHONG_ILLUMINATION_MODEL_H
#define BLINN_PHONG_ILLUMINATION_MODEL_H

#include "IShaderModule.h"

class BlinnPhongIlluminationModel : public IShaderModule
{
  public:
    BlinnPhongIlluminationModel();
    std::string getUniformDeclarations() const override;
    std::string getFunctionImplementation() const override;
    std::string getFunctionCall() const override;
    void setupUniforms(uint shaderProgram) const override;
};

#endif // BLINN_PHONG_ILLUMINATION_MODEL_H
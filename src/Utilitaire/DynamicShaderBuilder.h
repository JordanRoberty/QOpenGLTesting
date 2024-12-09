#ifndef DYNAMIC_SHADER_BUILDER_H
#define DYNAMIC_SHADER_BUILDER_H

#include <string>
#include <vector>
#include <memory>
#include <regex>

#include "IShaderModule.h"

class DynamicShaderBuilder
{
  public:
    DynamicShaderBuilder();
    void setBaseTemplate(const std::string &templateSource);
    void setIlluminationModel(std::unique_ptr<IShaderModule> model);
    void addEffect(std::unique_ptr<IShaderModule> effect);
    std::string readShaderFile(const std::string &filePath);

    std::string generateShaderSource() const;
    void setupUniforms(uint shaderProgram);
  
  private:
    std::string m_baseShaderTemplate;
     std::unique_ptr<IShaderModule> m_illuminationModel = nullptr;
    std::vector<std::unique_ptr<IShaderModule>> m_effects;
};

#endif // DYNAMIC_SHADER_BUILDER_H
#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader();
    ~Shader();
    
    bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    bool CompileFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    
    void Use() const;
    
    void SetMat4(const std::string& name, const glm::mat4& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetInt(const std::string& name, int value) const;
    
    unsigned int GetProgramID() const { return programID; }
    
private:
    unsigned int programID;
    
    bool CompileShader(unsigned int shader, const std::string& source, const std::string& type);
    bool LinkProgram(unsigned int vertexShader, unsigned int fragmentShader);
};

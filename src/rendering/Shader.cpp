#include "Shader.h"
#include "../core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader()
    : programID(0)
{
}

Shader::~Shader() {
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);
    
    if (!vFile.is_open() || !fFile.is_open()) {
        Logger::Log(LogLevel::Error, "Failed to open shader files");
        return false;
    }
    
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();
    
    return CompileFromSource(vStream.str(), fStream.str());
}

bool Shader::CompileFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    if (!CompileShader(vertexShader, vertexSource, "VERTEX")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    if (!CompileShader(fragmentShader, fragmentSource, "FRAGMENT")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    if (!LinkProgram(vertexShader, fragmentShader)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

bool Shader::CompileShader(unsigned int shader, const std::string& source, const std::string& type) {
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        Logger::Log(LogLevel::Error, type + " shader compilation failed: " + infoLog);
        return false;
    }
    
    return true;
}

bool Shader::LinkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);
    
    int success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        Logger::Log(LogLevel::Error, "Shader program linking failed: " + infoLog);
        return false;
    }
    
    return true;
}

void Shader::Use() const {
    glUseProgram(programID);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

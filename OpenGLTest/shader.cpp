#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

std::string Shader::utilShaderText = "";
unsigned int Shader::currentlyUsed = 0;

// First part form https://learnopengl.com/Getting-started/Shaders
Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // Retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // Ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // Cose file handlers
        vShaderFile.close();
        fShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "The shader file couldn't be read!" << std::endl;
    }

    fragmentCode = GetUtilShaderText() + fragmentCode;

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // Create vertex shader obejct and compile
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    CheckShaderCompilation(vertexShader, vertexPath);

    // Create fragment shader obejct and compile
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    CheckShaderCompilation(fragmentShader, fragmentPath);

    // Create a shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    int success;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "Ooops.. the shader program creation failed\n" << infoLog << std::endl;
    }

    // Delete shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader() { ID = -1; }

void Shader::Use()
{
    if (currentlyUsed == ID) return;

    currentlyUsed = ID;
    glUseProgram(ID);
}

void Shader::CheckShaderCompilation(unsigned int shaderID, const char* path)
{
    int success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "Congratulations! The shader " << path << " couldn't be compiled!\n" << infoLog << std::endl;
    }
}

std::string Shader::GetUtilShaderText()
{
    if (utilShaderText != "") return utilShaderText;

    std::string path = "Shaders\\util.glsl";

    std::ifstream shaderFile;
    std::string shaderCodeStr;
    const char* shaderCode = nullptr;
    try 
    {
        // Open file
        shaderFile.open(path);
        std::stringstream shaderStream;
        // Read file's buffer contents into streams
        shaderStream << shaderFile.rdbuf();
        // Cose file handlers
        shaderFile.close();
        // Convert stream into string
        shaderCodeStr = shaderStream.str();
        shaderCode = shaderCodeStr.c_str();
    }
    catch (std::ifstream::failure e) 
    {
        std::cout << "Util shader file couldn't be read!" << std::endl;
    }

    if (shaderCode == nullptr)
        return "";

    utilShaderText = shaderCode;
    return shaderCode;
}

void Shader::SetUniform(const char* uniformName, bool value)
{
    Use();
    glUniform1i(glGetUniformLocation(ID, uniformName), value);
}

void Shader::SetUniform(const char* uniformName, int value)
{
    Use();
    glUniform1i(glGetUniformLocation(ID, uniformName), value);
}

void Shader::SetUniform(const char* uniformName, float value)
{
    Use();
    glUniform1f(glGetUniformLocation(ID, uniformName), value);
}

void Shader::SetUniform(const char* uniformName, glm::vec4 value)
{
    Use();
    glUniform4f(glGetUniformLocation(ID, uniformName), value.x, value.y, value.z, value.w);
}

void Shader::SetUniform(const char* uniformName, glm::vec2 value)
{
    Use();
    glUniform2f(glGetUniformLocation(ID, uniformName), value.x, value.y);
}

void Shader::SetUniform(const char* uniformName, glm::mat4 trans) 
{
    Use();
    glUniformMatrix4fv(glGetUniformLocation(ID, uniformName), 1, GL_FALSE, glm::value_ptr(trans));
}

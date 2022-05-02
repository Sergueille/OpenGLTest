#include "Sprite.h"

#include <iostream>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"
#include "mesh.h"
#include "shader.h"
#include "Utility.h"
#include "Camera.h"
#include "EventManager.h"
#include "RessourceManager.h"
#include "LightManager.h"

std::priority_queue<Sprite*, std::vector<Sprite*>, CompareSprite> Sprite::drawQueue = std::priority_queue<Sprite*, std::vector<Sprite*>, CompareSprite>();

Sprite::Sprite(Texture* texture, glm::vec3 position, glm::vec2 size, float rotate, glm::vec4 color, Shader* shader, bool isUI)
{
    this->texture = texture;
    this->position = position;
    this->size = size;
    this->rotate = rotate;
    this->color = color;
    this->isUI = isUI;
    this->shader = shader;
    this->UVStart = glm::vec2(0, 0);
    this->UVEnd = glm::vec2(1, 1);
    this->forceOpaque = false;
    this->setUniforms = nullptr;
    this->setUniformsObjectCall = nullptr;
    this->isLit = false;

    this->isDrawnOnMainLoop = false;

    this->autoDrawFunc = nullptr;
}

Sprite::Sprite(bool isUI, glm::vec3 position, glm::vec2 size, glm::vec4 color) :
    Sprite(nullptr, position, size, 0.f, color, nullptr, isUI)
{ }

Sprite::Sprite(glm::vec3 start, glm::vec3 end, glm::vec4 color) :
    Sprite(nullptr, glm::vec3(0), glm::vec2(0), 0.f, color, nullptr, true)
{
    this->position = (start + end) / 2.f;
    this->size = end - start;
}

void Sprite::Draw()
{
    if (IsTransparent())
    {
        Sprite* copy = new Sprite(*this);
        copy->isDrawnOnMainLoop = false;
        drawQueue.push(copy);
    }
    else
    {
        DrawNow();
    }
}

Sprite::~Sprite()
{
    if (isDrawnOnMainLoop)
        StopDrawing();
}

void Sprite::DrawNow()
{
    Shader* realShader;
    if (shader == nullptr)
    {
        realShader = &RessourceManager::shaders["sprite"];
    }
    else
    {
        realShader = shader;
    }

    // No texture? ratio of 1
    float ratio = texture == nullptr ? 1 : texture->ratio;

    realShader->Use(); // TODO optimise if shader already used
    glm::mat4 transform = glm::mat4(1.0f); // Transformation matrix
    transform = glm::translate(transform, position); // Translate
    // transform = glm::translate(transform, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // Translate to pivot // TODO: chage pivot
    transform = glm::rotate(transform, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate
    // transform = glm::translate(transform, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); // Translate back to center
    transform = glm::scale(transform, glm::vec3(size, 1.0f) * glm::vec3(ratio, 1, 1)); // Scale with size and texture ratio

    realShader->SetUniform("transform", transform); // Set transformation matrix to shader
    if (isUI)
        realShader->SetUniform("projection", Camera::GetUIProjection()); // Set projection matrix to shader
    else
        realShader->SetUniform("projection", Camera::GetOrthographicProjection()); // Set projection matrix to shader
    realShader->SetUniform("mainColor", color); // Set color

    // Set texture if it exists
    if (texture != nullptr)
    {
        realShader->SetUniform("hasTexture", true);
        realShader->SetUniform("mainTexture", 0);
        texture->Use(0);
    }
    else
    {
        realShader->SetUniform("hasTexture", false);
    }

    if (isLit)
    {
        realShader->SetUniform("isLit", true);
        realShader->SetUniform("lightmapStart", LightManager::lightmapMin);
        realShader->SetUniform("lightmapEnd", LightManager::lightmapMax);
        realShader->SetUniform("lightmap", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, LightManager::GetLightData());
    }
    else
    {
        realShader->SetUniform("isLit", false);
    }

    realShader->SetUniform("UVstart", UVStart);
    realShader->SetUniform("UVend", UVEnd);

    if (setUniforms != nullptr)
    {
        setUniforms(realShader, setUniformsObjectCall);
    }

    // Get mesh and draw
    SpriteRenderer::GetMesh();
    SpriteRenderer::mesh->DrawMesh();

    glActiveTexture(GL_TEXTURE0); // Make sure stays 0
}

void Sprite::DrawAll()
{
    int i = 0;
    while (!drawQueue.empty())
    {
        i++;
        drawQueue.top()->DrawNow();
        delete drawQueue.top();
        drawQueue.pop();
    }
}

Sprite* Sprite::Copy()
{
    Sprite* copy = new Sprite(*this);

    if (copy->isDrawnOnMainLoop)
    {
        copy->isDrawnOnMainLoop = false;
        copy->DrawOnMainLoop();
    }

    return copy;
}

void Sprite::DrawOnMainLoop()
{
    if (!isDrawnOnMainLoop)
    {
        autoDrawFunc = EventManager::OnMainLoop.push_end([this] { this->Draw(); });
        isDrawnOnMainLoop = true;
    }
}

void Sprite::StopDrawing()
{
    if (isDrawnOnMainLoop)
    {
        EventManager::OnMainLoop.remove(autoDrawFunc);
        isDrawnOnMainLoop = false;
    }
}

bool Sprite::IsTransparent()
{
    bool opaqueTex = texture == nullptr || texture->isOpaque;
    return (!opaqueTex || color.a < 1 || shader != nullptr) && !forceOpaque;
}

Mesh* SpriteRenderer::mesh = nullptr;

Mesh* SpriteRenderer::GetMesh()
{
	if (mesh != NULL) return mesh;

    float vertices[] = {
        0.5f,   0.5f,   0.0f,  1.0f, 1.0f,
        0.5f,   -0.5f,  0.0f,  1.0f, 0.0f,
        -0.5f,  -0.5f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    mesh = new Mesh(&vertices[0], (int)std::size(vertices), &indices[0], (int)std::size(indices));
    return mesh;
}

bool CompareSprite::operator()(Sprite* lhs, Sprite* rhs)
{
    return lhs->position.z > rhs->position.z;
}

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

std::priority_queue<Sprite*, std::vector<Sprite*>, CompareSprite> Sprite::drawQueue = std::priority_queue<Sprite*, std::vector<Sprite*>, CompareSprite>();

Sprite::Sprite(Texture* texture, glm::vec3 position, glm::vec2 size, float rotate, glm::vec4 color, Shader* shader, bool isUI, bool isTransparent, bool destroyAfterDrawing)
{
    this->texture = texture;
    this->position = position;
    this->size = size;
    this->rotate = rotate;
    this->color = color;
    this->isUI = isUI;
    this->shader = shader;
    this->destroyAfterDrawing = destroyAfterDrawing;

    if (texture == nullptr && color.a == 1)
        this->isTransparent = false;
    else
        this->isTransparent = isTransparent;

    // TODO: make auto-subscribe optional
    EventManager::OnMainLoop.push_back([this] { this->Draw(); });
}

Sprite::Sprite(bool isUI, glm::vec3 position, glm::vec2 size, glm::vec4 color, bool isTransparent, bool destroyAfterDrawing) :
    Sprite(nullptr, position, size, 0.f, color, nullptr, isUI, isTransparent, destroyAfterDrawing)
{ }

Sprite::Sprite(glm::vec3 start, glm::vec3 end, glm::vec4 color) :
    Sprite(nullptr, glm::vec3(0), glm::vec2(0), 0.f, color, nullptr, true, false, true)
{
    this->position = (start + end) / 2.f;
    this->size = end - start;
    this->isTransparent = color.a < 1;
}

void Sprite::Draw()
{
    if (isTransparent)
    {
        drawQueue.push(this);
    }
    else
    {
        DrawNow();
    }
}

void Sprite::DrawNow()
{
    if (shader == nullptr)
    {
        if (texture == nullptr)
            shader = &RessourceManager::shaders["spriteColor"];
        else
            shader = &RessourceManager::shaders["sprite"];
    }
    else
    {
        shader = shader;
    }

    // No texture? ratio of 1
    float ratio = texture == nullptr ? 1 : texture->ratio;

    shader->Use(); // TODO optimise if shader already used
    glm::mat4 transform = glm::mat4(1.0f); // Transformation matrix
    transform = glm::translate(transform, position); // Translate
    // transform = glm::translate(transform, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f)); // Translate to pivot //TODO: chage pivot
    transform = glm::rotate(transform, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate
    // transform = glm::translate(transform, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f)); //  Translate back to center
    transform = glm::scale(transform, glm::vec3(size, 1.0f) * glm::vec3(ratio, 1, 1)); // Scale with size and texture ratio

    shader->SetUniform("transform", transform); // Set transformation matrix to shader
    if (isUI)
        shader->SetUniform("projection", Camera::GetUIProjection()); // Set projection matrix to shader
    else
        shader->SetUniform("projection", Camera::GetOrthographicProjection()); // Set projection matrix to shader
    shader->SetUniform("mainColor", color); // Set color

    // Set texture if it exists
    if (texture != nullptr)
    {
        shader->SetUniform("mainTexture", 0);
        texture->Use(0);
    }

    // Get mesh and draw
    SpriteRenderer::GetMesh();
    SpriteRenderer::mesh->DrawMesh();
}

Sprite::~Sprite()
{
    std::cout << "Here!" << std::endl;
}

void Sprite::DrawAll()
{
    while (!drawQueue.empty())
    {
        if (drawQueue.top() == nullptr)
        {
            std::cout << "Sprite subscribed to transparent draw queue but have been destroyed!" << std::endl;
            std::cout << "When creating a sprite, always store it in the heap and make sure auto-destroy property is true" << std::endl;
        }
        else
        {
            // Draw the sprite
            drawQueue.top()->DrawNow();

            // Auto-delete
            if (drawQueue.top()->destroyAfterDrawing)
                delete drawQueue.top();
        }

        drawQueue.pop();
    }
}

namespace SpriteRenderer
{
    Mesh* GetMesh()
    {
	    if (mesh != NULL) return mesh;

        float vertices[] = {
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        };

        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        mesh = new Mesh(&vertices[0], (int)std::size(vertices), &indices[0], (int)std::size(indices));
        return mesh;
    }
}


bool CompareSprite::operator()(const Sprite& lhs, const Sprite& rhs)
{
    return lhs.position.z < rhs.position.z;
}

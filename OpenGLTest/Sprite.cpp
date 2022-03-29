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


Sprite::Sprite(Texture* texture, glm::vec3 position, glm::vec2 size, float rotate, glm::vec4 color, Shader* shader)
{
    this->texture = texture;

    if (shader == nullptr)
    {
        if (texture == nullptr)
            this->shader = &RessourceManager::shaders["spriteColor"];
        else
            this->shader = &RessourceManager::shaders["sprite"];
    }
    else 
    {
        this->shader = shader;
    }

    this->position = position;
    this->size = size;
    this->rotate = rotate;
    this->color = color;

    EventManager::OnMainLoop.push_back([this] { this->Draw(); });
}

void Sprite::Draw()
{
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


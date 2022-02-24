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

Sprite::Sprite(Shader* shader, Texture* texture, glm::vec3 position, glm::vec2 size, float rotate, glm::vec4 color)
{
    this->shader = shader;
    this->texture = texture;
    this->position = position;
    this->size = size;
    this->rotate = rotate;
    this->color = color;
}

void Sprite::Draw()
{
    shader->Use(); // TODO optimise if shader already used
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::translate(transform, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    transform = glm::rotate(transform, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::translate(transform, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    transform = glm::scale(transform, glm::vec3(size, 1.0f) * glm::vec3(texture->ratio, 1, 1));

    shader->SetUniform("transform", transform);
    shader->SetUniform("projection", Camera::GetOrthographicProjection());
    shader->SetUniform("spriteColor", color);
    shader->SetUniform("mainTexture", 0);

    texture->Use(0);

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


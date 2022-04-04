/*
Code from https://learnopengl.com/Getting-started/Hello-Triangle
*/

#include <iostream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "mesh.h"

using namespace std;

Mesh::Mesh(float* firstV, int vcount, unsigned int* firstID, int icount)
{
    this->verticesData = firstV;
    this->vcount = vcount;
    this->indices = firstID;
    this->icount = icount;
    SetVertexData();
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    // TODO: where are the EBO and VBO destroyed?
}

void Mesh::DrawMesh()
{    
    glBindVertexArray(VAO); // Use the vertex array
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw mesh!
    glBindVertexArray(0);
}

void Mesh::SetVertexData()
{
    // Create vertex buffer
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    // Create vertex array
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create element buffer
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    // Set vertex data in buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vcount, verticesData, GL_STATIC_DRAW);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set indices in buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * icount, indices, GL_STATIC_DRAW);

    this->VAO = VAO;
}

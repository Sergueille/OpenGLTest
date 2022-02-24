#pragma once

#include "mesh.h"

class Mesh
{
public:
    float* verticesData;
    int vcount;
    unsigned int* indices;
    int icount;

    unsigned int VAO;

    Mesh(float* firstV, int vcount, unsigned int* firstID, int icount);
    void DrawMesh();

private:
    void SetVertexData();
};

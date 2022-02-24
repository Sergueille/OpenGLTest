#pragma once

struct MeshVertices
{
    float* verticesData;
    int vcount;
    unsigned int* indices;
    int icount;

    MeshVertices(float* firstV, int vcount, unsigned int* firstID, int icount);
};

class Mesh
{
public:
    unsigned int VAO;
    unsigned int shaderID;

    Mesh(MeshVertices* vertices, unsigned int shaderID);
    void DrawMesh();

private:
    void SetVertexData(MeshVertices* data);
};

#ifndef MESH____H
#define MESH____H

#include <GL/glew.h>

// CreateMesh()    -> position only            (starter default)
// CreateMeshLit() -> position + normal + colour+skyBlend
class Mesh
{
    public:
        Mesh();
        ~Mesh();

        void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
        void CreateMeshLit(const GLfloat* vertices, const unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
        void RenderMesh();
        void ClearMesh();

    private:
        GLuint VAO, VBO, IBO;
        GLsizei indexCount;
};

#endif

#ifndef GLTFLOADER_H
#define GLTFLOADER_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "tiny_gltf.h"

class GLTFLoader : protected QOpenGLFunctions
{
  public:

    // --- constructor and destructor ---
    GLTFLoader(QOpenGLFunctions *glFuncs);
    ~GLTFLoader();


    // Load a glTF model from a file, it can be either a .glb or .gltf file
    bool loadModel(const QString &filename);
    void render(QOpenGLShaderProgram* shaderProgram, const QMatrix4x4& projection, const QMatrix4x4& view);
    void cleanUp();

    enum class TextureType
    {
      Texture1D,
      Texture2D
    };

    struct TextureInfo
    {
      QOpenGLTexture *texture;
      TextureType type;
    };

    // The Mesh struct is used to store the vertex and index data of a mesh
    struct Mesh {
      QOpenGLBuffer vbo;
      QOpenGLBuffer ebo;
      GLuint displayListId;
      int indexCount; // May be useful if we use VAOs
      QMatrix4x4 modelMatrix;
      std::vector<TextureInfo> textureInfos; // 0: base color, 1: normal map, 2: metallic-roughness


      Mesh(): vbo(QOpenGLBuffer(QOpenGLBuffer::VertexBuffer)), 
              ebo(QOpenGLBuffer(QOpenGLBuffer::IndexBuffer)), 
              displayListId(0), indexCount(0) 
              {}
    };

    std::vector<Mesh> m_meshes;




  private:

    // The Vertex struct is used to store the vertex data of a mesh
    struct Vertex {
        QVector3D position;
        QVector3D normal;
        QVector3D color;
        QVector2D texCoords;
    };

    // Retrieve the vertex and index data from a mesh primitive and build a Mesh object from it
    bool setUpMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const tinygltf::Primitive &primitive, const QMatrix4x4 &transform);

    // Recursively process all nodes in the glTF model
    void processNode(const tinygltf::Model &model, const tinygltf::Node &node, const QMatrix4x4 &parentTransform);

    void centerModel(std::vector<Vertex>& vertices);

    tinygltf::Model m_model;
    QOpenGLFunctions *m_glFuncs;

};

#endif // GLTFLOADER_H
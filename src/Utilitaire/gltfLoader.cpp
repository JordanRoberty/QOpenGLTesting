#include <cstddef>
#include <sys/types.h>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gltfLoader.h"
#include <iostream>
#include <webp/decode.h>


GLTFLoader::GLTFLoader(QOpenGLFunctions *glFuncs)
    : m_glFuncs(glFuncs)
{
}

GLTFLoader::~GLTFLoader()
{
  cleanUp();
}

bool LoadWebPOrDefaultImage(tinygltf::Image* image, int image_idx, std::string* err, std::string* warn,
                    int req_width, int req_height, const unsigned char* data, int size, void* user_data) {
        // Vérifier si c'est un WebP
    if (WebPGetInfo(data, size, nullptr, nullptr)) {
        WebPDecoderConfig config;
        if (!WebPInitDecoderConfig(&config)) {
            if (err) *err = "Failed to init WebP config";
            return false;
        }

        // Vérifier les features de l'image
        if (WebPGetFeatures(data, size, &config.input) != VP8_STATUS_OK) {
            if (err) *err = "Failed to get WebP features";
            return false;
        }

        // Configurer le décodeur
        config.options.use_threads = 1;  // Utiliser le multi-threading
        config.output.colorspace = MODE_RGBA;
        
        // Allouer le buffer
        std::vector<uint8_t> output_buffer(
            config.input.width * config.input.height * 4);
        config.output.u.RGBA.rgba = output_buffer.data();
        config.output.u.RGBA.stride = config.input.width * 4;
        config.output.u.RGBA.size = output_buffer.size();

        // Décoder
        if (WebPDecode(data, size, &config) != VP8_STATUS_OK) {
            if (err) *err = "Failed to decode WebP";
            return false;
        }

        // Remplir l'image
        image->width = config.input.width;
        image->height = config.input.height;
        image->component = 4;
        image->image = std::move(output_buffer);

        WebPFreeDecBuffer(&config.output);
        return true;
    }

    // Sinon utiliser le chargeur par défaut
    return tinygltf::LoadImageData(image, image_idx, err, warn, 
                                 req_width, req_height, data, size, user_data);
}

bool GLTFLoader::loadModel(const QString &filename)
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    QOpenGLContext* currentContext = QOpenGLContext::currentContext();
    if (!currentContext) {
        qDebug() << "No current OpenGL context";
        return false;
    }

    loader.SetImageLoader(LoadWebPOrDefaultImage, nullptr);

    // glTF files can be either binary (.glb) or ASCII (.gltf), so we need to check the file extension
    bool ret = filename.endsWith(".glb") ? 
      loader.LoadBinaryFromFile(&m_model, &err, &warn, filename.toStdString()) :
      loader.LoadASCIIFromFile(&m_model, &err, &warn, filename.toStdString());

    if (!warn.empty()) {
        qDebug() << "GLTF Warning: " << QString::fromStdString(warn);
    }

    if (!err.empty()) {
        qDebug() << "GLTF Error: " << QString::fromStdString(err);
    }

    if (!ret) {
        qDebug() << "Failed to load glTF file";
        return false;
    }

    const tinygltf::Scene &scene = m_model.scenes[m_model.defaultScene];
    for(size_t i=0; i<scene.nodes.size(); i++) {
       // std::cout << "Node: " << m_model.nodes[scene.nodes[i]].name << std::endl;
        processNode(m_model, m_model.nodes[scene.nodes[i]], QMatrix4x4());
    }

    return true;
}

void GLTFLoader::processNode(const tinygltf::Model &model, const tinygltf::Node &node, const QMatrix4x4 &parentTransform)
{
  QMatrix4x4 nodeTransform = parentTransform;
  QMatrix4x4 localTransform;
  localTransform.setToIdentity();

  // Apply node transformations
  if (node.matrix.size() == 16)
  {
    for (int i = 0; i < 16; i++) {
      localTransform(i / 4, i % 4) = node.matrix[i];
    }
  }
  else
  {
    // Handle TRS (Translation, Rotation, Scale) properties
    if (node.translation.size() == 3) {
        localTransform.translate(node.translation[0], node.translation[1], node.translation[2]);
    }
    if (node.rotation.size() == 4) {
        QQuaternion rotation(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        localTransform.rotate(rotation);
    }
    if (node.scale.size() == 3) {
        localTransform.scale(node.scale[0], node.scale[1], node.scale[2]);
    }
  }

  nodeTransform *= localTransform;

  // Process mesh if present
  if(node.mesh >= 0)
  {
    const tinygltf::Mesh &mesh = model.meshes[node.mesh];
    for(const auto &primitive : mesh.primitives)
    {
      if(!setUpMesh(model, mesh, primitive, nodeTransform))
      {
        qDebug() << "Failed to set up mesh";
      }
    }
  }

  // Process children
  for(size_t i=0; i<node.children.size(); i++)
  {
    processNode(model, model.nodes[node.children[i]], nodeTransform);
  }
}

void GLTFLoader::centerModel(std::vector<Vertex>& vertices) {
    // Calcul de la bounding box
    float max = std::numeric_limits<float>::max();
    float min = std::numeric_limits<float>::lowest();
    QVector3D minBounds(max, max, max);
    QVector3D maxBounds(min, min, min);

    
    // Trouver les points min et max
    for (const auto& vertex : vertices) {
        // minBounds = QVector3D::minimum(minBounds, vertex.position);
        // maxBounds = QVector3D::maximum(maxBounds, vertex.position);
        minBounds = QVector3D(
        std::min(minBounds.x(), vertex.position.x()),
        std::min(minBounds.y(), vertex.position.y()),
        std::min(minBounds.z(), vertex.position.z())
    );
    maxBounds = QVector3D(
        std::max(maxBounds.x(), vertex.position.x()),
        std::max(maxBounds.y(), vertex.position.y()),
        std::max(maxBounds.z(), vertex.position.z())
    );
    }
    
    // Calculer le centre
    QVector3D center = (minBounds + maxBounds) * 0.5f;

    QMatrix4x4 translation;
    translation.setToIdentity();
    //translation.translate(0.0f, 0.0f, 5.0f);

    QMatrix4x4 scale;
    scale.setToIdentity();
    scale.scale(0.05f);
    
    // Translater tous les vertices
    for (auto& vertex : vertices) {
        vertex.position -= center;
        vertex.position = scale * translation * vertex.position;
    }
}

bool GLTFLoader::setUpMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive, const QMatrix4x4& transform) {
  if (primitive.indices < 0 || primitive.attributes.find("POSITION") == primitive.attributes.end())
  {
      qDebug() << "Primitive is missing indices or position attribute";
      return false;
  }

  Mesh glMesh;
  std::vector<Vertex> vertices;

  // Get accessor and fill vertex data

  // POSITIONS
  {
  const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
  const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
  const float* positions = reinterpret_cast<const float*>(&model.buffers[posView.buffer].data[posView.byteOffset + posAccessor.byteOffset]);
  for (size_t i = 0; i < posAccessor.count; i++) {
      Vertex vertex;
      vertex.position = QVector3D(
          positions[i * 3],
          positions[i * 3 + 1],
          positions[i * 3 + 2]
      );
      vertices.push_back(vertex);
  }

  centerModel(vertices);
  }

  // NORMALS
  if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
  {
    const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
    const tinygltf::BufferView& normalView = model.bufferViews[normalAccessor.bufferView];
    const float* normals = reinterpret_cast<const float*>(&model.buffers[normalView.buffer].data[normalView.byteOffset + normalAccessor.byteOffset]);
    for (size_t i = 0; i < normalAccessor.count; i++) {
        vertices[i].normal = QVector3D(
            normals[i * 3],
            normals[i * 3 + 1],
            normals[i * 3 + 2]
        );
    }
  }
  else
  {
    std::cout << "No normal provided" << std::endl;
    // Assign a default normal if none is provided
    for(auto& vertex : vertices)
    {
      vertex.normal = QVector3D(0.0f, 0.0f, 1.0f);
    }
  }

  // UV
  if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
  {
    const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
    const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
    const float* uvs = reinterpret_cast<const float*>(&model.buffers[uvView.buffer].data[uvView.byteOffset + uvAccessor.byteOffset]);

    if(uvAccessor.type == TINYGLTF_TYPE_SCALAR)
    {
      for(size_t i = 0; i < uvAccessor.count; i++)
      {
        float u = uvs[i];
        float v = u;
        vertices[i].texCoords = QVector2D(u, v);
      }
    }
    else if(uvAccessor.type == TINYGLTF_TYPE_VEC2)
    {
      for (size_t i = 0; i < uvAccessor.count; i++) {
        vertices[i].texCoords = QVector2D(
            uvs[i * 2],
            uvs[i * 2 + 1]
        );
      }
    }
  }
  else
  {
    std::cout << "No UV provided" << std::endl;
  }

  if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) 
  {
      const tinygltf::Accessor& colorAccessor = model.accessors[primitive.attributes.at("COLOR_0")];
      const tinygltf::BufferView& colorView = model.bufferViews[colorAccessor.bufferView];
      const float* colors = reinterpret_cast<const float*>(&model.buffers[colorView.buffer].data[colorView.byteOffset + colorAccessor.byteOffset]);

      // fill vertex data with colors
      for (size_t i = 0; i < colorAccessor.count; i++) {
          vertices[i].color = QVector3D(
              colors[i * 3],
              colors[i * 3 + 1],
              colors[i * 3 + 2]
          );
      }
  }
  // else check for material color   
  else if (primitive.material >= 0)
  {

    const tinygltf::Material& material = model.materials[primitive.material];
    
    // Texture handling
    if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
      const tinygltf::Texture &tex = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
      
      // Try to get image index, either from regular source or WebP extension
      int imageIndex = tex.source;
      
      // If regular source is not valid, check WebP extension
      if (imageIndex < 0 && tex.extensions.find("EXT_texture_webp") != tex.extensions.end()) {
          auto& ext = tex.extensions.at("EXT_texture_webp");
          if (ext.IsObject() && ext.Has("source")) {
              imageIndex = ext.Get("source").Get<int>();
          }
      }

      TextureInfo textureInfo;
      
      if (imageIndex >= 0) {
        const tinygltf::Image &image = model.images[imageIndex];
        
        if(image.height == 1)
        {
          textureInfo.type = TextureType::Texture1D;

          QOpenGLTexture* texture = new QOpenGLTexture(QOpenGLTexture::Target1D);
          texture->setSize(image.width);
          texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
          texture->allocateStorage();
          texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, image.image.data());
          
          texture->setMinificationFilter(QOpenGLTexture::Linear);
          texture->setMagnificationFilter(QOpenGLTexture::Linear);
          texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::Repeat);

          
          textureInfo.texture = texture;
        }
        else
        {
          textureInfo.type = TextureType::Texture2D;

          QOpenGLTexture* texture = new QOpenGLTexture(QImage(
              image.image.data(),
              image.width,
              image.height,
              QImage::Format_RGBA8888
          ));
          
          texture->setMinificationFilter(QOpenGLTexture::Linear);
          texture->setMagnificationFilter(QOpenGLTexture::Linear);
          texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::Repeat);
          texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::Repeat);
          
          textureInfo.texture = texture;
        }
              
        //glMesh.textureInfos.resize(1);
        //glMesh.textureInfos[0] = textureInfo;
        glMesh.textureInfos.push_back(textureInfo);
      }
      else
      {
        std::cout << "Invalid image dimensions" << std::endl;
      }
    }
    
    // Base color handling
    if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
        QVector3D color(
            material.pbrMetallicRoughness.baseColorFactor[0],
            material.pbrMetallicRoughness.baseColorFactor[1],
            material.pbrMetallicRoughness.baseColorFactor[2]
        );
        
        for(auto& vertex : vertices)
        {
            vertex.color = color;
        }
    }
    else
    {
        // Material color not provided so use a default color
        for(auto& vertex : vertices)
        {
            vertex.color = QVector3D(0.8f, 0.8f, 0.8f);
        }
    }
}
else
{
    std::cout << "No color provided" << std::endl;
    // default color
    for(auto& vertex : vertices)
    {
        vertex.color = QVector3D(0.8f, 0.8f, 0.8f);
    }
}

  // Get indices
  const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
  const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];

  GLenum indexType = indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

  // Determine the type of the indices and cast accordingly
  if (indexType == GL_UNSIGNED_SHORT)
  {
    std::vector<uint16_t> indices16;
    const uint16_t* indices_ptr = reinterpret_cast<const uint16_t*>(&model.buffers[indexView.buffer].data[indexView.byteOffset + indexAccessor.byteOffset]);
    for (size_t i = 0; i < indexAccessor.count; i++)
    {
        indices16.push_back(indices_ptr[i]);
    }

    glMesh.ebo.create();
    glMesh.ebo.bind();
    glMesh.ebo.allocate(indices16.data(), indices16.size() * sizeof(uint16_t));
    glMesh.ebo.release();
    glMesh.indexCount = indexAccessor.count;

  }
  else if (indexType == GL_UNSIGNED_INT)
  {
    std::vector<uint32_t> indices32;
    const uint32_t* indices_ptr = reinterpret_cast<const uint32_t*>(&model.buffers[indexView.buffer].data[indexView.byteOffset + indexAccessor.byteOffset]);
    for (size_t i = 0; i < indexAccessor.count; i++)
    {
        indices32.push_back(indices_ptr[i]);
    }
    
    glMesh.ebo.create();
    glMesh.ebo.bind();
    glMesh.ebo.allocate(indices32.data(), indices32.size() * sizeof(uint32_t));
    glMesh.ebo.release();        
    glMesh.indexCount = indexAccessor.count;

  }
  else
  {
    qDebug() << "Unsupported index component type";
    return false;
  }
  
  // Create and set-up Buffers and Display List
  glMesh.vbo.create();
  glMesh.vbo.bind();
  glMesh.vbo.allocate(vertices.data(), vertices.size() * sizeof(Vertex));
  glMesh.vbo.release();

  glMesh.displayListId = glGenLists(1);
  glNewList(glMesh.displayListId, GL_COMPILE);

  glMesh.vbo.bind();
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));

  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

  glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));

  glMesh.ebo.bind();
  m_glFuncs->glDrawElements(GL_TRIANGLES, glMesh.indexCount, indexType, 0);
  glMesh.ebo.release();

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glMesh.vbo.release();

  glEndList();


  glMesh.modelMatrix = transform;

  // Store the mesh
  m_meshes.push_back(glMesh);

  return true;
}

void GLTFLoader::render(QOpenGLShaderProgram* shaderProgram, const QMatrix4x4& projection, const QMatrix4x4& view)
{
  if(!shaderProgram || m_meshes.empty())
  {
    return;
  }

  shaderProgram->bind();
  shaderProgram->setUniformValue("u_projection", projection);
  shaderProgram->setUniformValue("u_view", view);

  for(const auto& mesh : m_meshes)
  {
    if(!mesh.textureInfos.empty())
    {
      shaderProgram->setUniformValue("u_hasTexture", true);

      for(size_t i = 0; i < mesh.textureInfos.size(); i++)
      {
        const auto& textureInfo = mesh.textureInfos[i];
        if(textureInfo.type == TextureType::Texture1D)
        {
          shaderProgram->setUniformValue("u_textureType", 0);
          textureInfo.texture->bind();
          shaderProgram->setUniformValue("u_texture1D", 0);
          shaderProgram->setUniformValue("u_texture2D", 1);
        }
        else
        {
          shaderProgram->setUniformValue("u_textureType", 1);
          textureInfo.texture->bind();
          shaderProgram->setUniformValue("u_texture2D", 0);
          shaderProgram->setUniformValue("u_texture1D", 1); 
        }
      }
    }
    else
    {
      shaderProgram->setUniformValue("u_hasTexture", false);
    }

    shaderProgram->setUniformValue("u_model", mesh.modelMatrix);
    glCallList(mesh.displayListId);

    if(!mesh.textureInfos.empty())
    {
      for(size_t i = 0; i < mesh.textureInfos.size(); i++)
      {
        mesh.textureInfos[i].texture->release();
      }
    }
  }

  shaderProgram->release();
}

void GLTFLoader::cleanUp()
{
  for(auto& mesh : m_meshes)
  {
    glDeleteLists(mesh.displayListId, 1);
    mesh.vbo.destroy();
    mesh.ebo.destroy();

    for(auto& textureInfo : mesh.textureInfos)
    {
      textureInfo.texture->destroy();
    }
  }

  m_meshes.clear();
}
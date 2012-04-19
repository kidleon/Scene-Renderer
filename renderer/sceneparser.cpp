#include "sceneparser.h"

#include "keyvalues.h"
#include "light.h"
#include "material.h"
#include "mesh.h"
#include "meshloaderoff.h"
#include "meshloaderply.h"
#include "object.h"
#include "scene.h"
#include "texture.h"

Scene *SceneParser::load(const char *filename)
{
  KeyValues *data = new KeyValues();
  if(!data->loadFile(filename)) {
    fprintf(stderr, "Failed to load Scene");
    return NULL;
  }

  // TODO: should we have an internal object for the parser?
  setPath(filename);

  return loadScene(data);
}

Scene *SceneParser::loadScene(KeyValues *data)
{
  Scene *scene = new Scene();

  KeyValues *key = data->firstSubKey();
  while(key) {
    const char *name = key->name();

    if(strcmp(name, "object") == 0) {
      Object *object = loadObject(key);
      if(object) {
        scene->addObject(object);
      }
      else {
        fprintf(stderr, "Failed to load Object");
      }
    }
    else if(strcmp(name, "light") == 0) {
      Light *light = loadLight(key);
      if(light) {
        scene->addLight(light);
      }
      else {
        fprintf(stderr, "Failed to load Light");
      }
    }
    else {
      fprintf(stderr, "Unknown element: %s", name);
    }

    key = key->nextKey();
  }

  return scene;
}

Object *SceneParser::loadObject(KeyValues *data)
{
  const char *format = data->getString("format");
  if(!format) {
    fprintf(stderr, "Key 'format' not found on Object");
    return NULL;
  }

  Mesh *mesh;
  if(strcmp("OFF", format) == 0 || strcmp("PLY", format) == 0) {
    const char *file = data->getString("file");
    if(!file) {
      fprintf(stderr, "Key 'file' not found on Object");
      return NULL;
    }
    char *filename = resolvePath(file);

    MeshData *meshData;

    if(strcmp("OFF", format) == 0) {
      meshData = MeshLoaderOFF::load(filename);
    }
    else if(strcmp("PLY", format) == 0) {
      meshData = MeshLoaderPLY::load(filename);
    }

    if(!meshData) {
      fprintf(stderr, "Failed to load MeshData of %s", filename);
      delete[] filename;
      return NULL;
    }

    if(data->getInt("normalize", 1)) {
      meshData->normalize();
    }
    if(data->getInt("normals", 0)) {
      meshData->computeNormals();
    }

    mesh = new Mesh(meshData);

    delete meshData;
    delete[] filename;
  }
  else {
    fprintf(stderr, "Invalid object format: %s", format);
    return NULL;
  }

  Material *material = NULL;
  QGLShaderProgram *shaderProgram = new QGLShaderProgram();

  KeyValues *key;

  key = data->firstSubKey();
  while(key) {
    if(strcmp(key->name(), "shader") == 0) {
      QGLShader *shader = loadShader(key);
      if(shader) {
        shaderProgram->addShader(shader);
      }
      else {
        fprintf(stderr, "Failed to load shader");
      }
    }
    else if (strcmp(key->name(), "material") == 0) {
      if(material) {
        fprintf(stderr, "Duplicated material definition");
      }
      else {
        material = loadMaterial(key);
      }
    }
    key = key->nextKey();
  }

  if(!shaderProgram->link()) {
    fprintf(stderr, "Failed to link shader program");
  }

  Object *object = new Object(mesh, shaderProgram, material);

  key = data->firstSubKey();
  while(key) {
   if (strcmp(key->name(), "texture") == 0) {
      fprintf(stderr, "Loading texture");
      key->print(stderr);
    }
    key = key->nextKey();
  }

  return object;
}

QGLShader *SceneParser::loadShader(KeyValues *data)
{
  const char *type = data->getString("type");
  if(!type) {
    fprintf(stderr, "Key 'type' not found on Shader");
    return NULL;
  }

  const char *file = data->getString("file");
  if(!file) {
    fprintf(stderr, "Key 'file' not found on Shader");
    return NULL;
  }
  char *filename = resolvePath(file);

  QGLShader::ShaderType shaderType;
  if(strcmp(type, "vertex") == 0) {
    shaderType = QGLShader::Vertex;
  }
  else if(strcmp(type, "fragment") == 0) {
    shaderType = QGLShader::Fragment;
  }
  else {
    fprintf(stderr, "Invalid shader type: %s", type);
    delete[] filename;
    return NULL;
  }

  QGLShader *shader = new QGLShader(shaderType);
  if(!shader->compileSourceFile(filename)) {
    fprintf(stderr, "Failed to load shader %s", filename);
    fprintf(stdout, shader->log().toStdString().c_str());
    delete shader;
    delete[] filename;
    return NULL;
  }
  delete[] filename;

  return shader;
}

Material *SceneParser::loadMaterial(KeyValues *data)
{
  const char* ambient = data->getString("ambient");
  const char* diffuse = data->getString("diffuse");
  const char* specular = data->getString("specular");

  return new Material(strtoV4D(ambient), strtoV4D(diffuse), strtoV4D(specular));
}
Texture *SceneParser::loadTexture(KeyValues *data)
{
  return NULL;
}

Light *SceneParser::loadLight(KeyValues *data)
{
  const char* position = data->getString("position");
  if(!position) {
    fprintf(stderr, "Key 'position' not found on Light");
    return NULL;
  }

  const char* brightness = data->getString("brightness");
  if(!brightness) {
    fprintf(stderr, "Key 'brightness' not found on Light");
    return NULL;
  }

  return new Light(strtoV3D(position), strtoV4D(brightness));
}

QVector3D SceneParser::strtoV3D(const char *str)
{
  if(!str) {
    return QVector3D();
  }

  float x, y, z;
  sscanf(str, "%f %f %f", &x, &y, &z);
  return QVector3D(x, y, z);
}

QVector4D SceneParser::strtoV4D(const char *str)
{
  if(!str) {
    return QVector4D();
  }

  float x, y, z, w;
  sscanf(str, "%f %f %f %f", &x, &y, &z, &w);
  return QVector4D(x, y, z, w);
}

char *SceneParser::currentPath = NULL;

void SceneParser::setPath(const char *filename)
{
  int len = strlen(filename);
  while(filename[len] != '/') {
    len--;
  }

  currentPath = new char[len + 1];
  for(int i = 0; i <= len; ++i) {
    currentPath[i] = filename[i];
  }
  currentPath[len + 1] = '\0';
}

char *SceneParser::resolvePath(const char *filename)
{
  bool relative = (filename[0] != '/' && filename[0] != ':');
  int len = strlen(filename);
  if(relative) {
    len += strlen(currentPath);
  }

  int offset = 0;
  char *ret = new char[len + 1];
  if(relative) {
    for(char *c = currentPath; *c; c++) {
      ret[offset++] = *c;
    }
  }
  strcpy(&(ret[offset]), filename);
  return ret;
}

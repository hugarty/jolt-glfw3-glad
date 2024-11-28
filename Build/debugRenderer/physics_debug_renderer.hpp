#ifndef PHYSICS_DEBUG_RENDERER_HPP
#define PHYSICS_DEBUG_RENDERER_HPP


#ifndef JPH_DEBUG_RENDERER
#define JPH_DEBUG_RENDERER
#endif

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>


extern long ID_TOP_MERMAO;

class PhysicsDebugRenderer final : public JPH::DebugRenderer {

public:
  PhysicsDebugRenderer();

  void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
  void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor,
                            ECastShadow inCastShadow = ECastShadow::Off) override;

  Batch CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount) override;
  Batch CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const JPH::uint32 *inIndices,
                                    int inIndexCount) override;

  void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds, float inLODScaleSq,
                            JPH::ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode,
                            ECastShadow inCastShadow, EDrawMode inDrawMode) override;
  void DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view &inString, JPH::ColorArg inColor,
                          float inHeight) override;


  GLFWwindow *window;
  unsigned int widthSize;
  unsigned int heightSize;
  unsigned int VBO, EBO, VAO;
  unsigned int shaderProgram;
};

class ThatIHaveToMake : public JPH::RefTarget<ThatIHaveToMake> {};

class TriangleData : public JPH::RefTargetVirtual, public ThatIHaveToMake {
public:
  
  TriangleData(const JPH::DebugRenderer::Triangle *triangles, int num_triangles) ;
  TriangleData(const JPH::DebugRenderer::Vertex *vertices, int num_vertices, const JPH::uint32 *indices, int num_indices);

  virtual void AddRef() override { ThatIHaveToMake::AddRef(); }
  virtual void Release() override { if (--mRefCount == 0) delete this; }

  int num_triangles;
  std::vector<float> triangle_vertices;
  std::vector<float> vertices;
  std::vector<JPH::uint32> indices;
  long idTriangulo = ++ID_TOP_MERMAO;
  bool uses_indices;
};

#endif // PHYSICS_DEBUG_RENDERER_HPP

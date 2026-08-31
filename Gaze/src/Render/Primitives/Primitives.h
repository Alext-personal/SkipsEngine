#pragma once
#include "Render/Mesh.h"
namespace Gaze {
    enum class PrimitiveType {
        Triangle, Quad, Cube
    };
    inline std::string PrimitiveTypeToString(PrimitiveType type) {
        switch (type)
        {
        case PrimitiveType::Triangle:
            return "Triangle";
        case PrimitiveType::Quad:
            return "Quad";
        case PrimitiveType::Cube:
            return "Cube";
        }
    }
    class Primitives {
    public:
        static MeshData LoadPrimitiveByType(PrimitiveType type) {
            switch (type)
            {
            case PrimitiveType::Triangle:
                return Triangle();
            case PrimitiveType::Quad:
                return Quad();
            case PrimitiveType::Cube:
                return Cube();
            }
        }
    private:
        static MeshData Triangle() {
            MeshData triangleData{};
            triangleData.bufferData[0].data = {
                // Position              // Normal           // TexCoord
                -0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
                 0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
                 0.0f,  0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   0.5f, 1.0f
            };
            triangleData.bufferData[0].layout.Add(AttributeDataType::Float3); // pos
            triangleData.bufferData[0].layout.Add(AttributeDataType::Float3); // normals
            triangleData.bufferData[0].layout.Add(AttributeDataType::Float2); //texcoords
            return triangleData;
        }
        static MeshData Quad() {
            MeshData quadData{};
            quadData.bufferData[0].data = {
                // Position              // Normal           // TexCoord
                -0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
                 0.5f, -0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
                -0.5f,  0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
                 0.5f,  0.5f, 0.0f,       0.0f, 0.0f, 1.0f,   1.0f, 1.0f
            };
            quadData.indices = {
                0,1,2,
                1,3,2
            };
            quadData.bufferData[0].layout.Add(AttributeDataType::Float3);//Pos
            quadData.bufferData[0].layout.Add(AttributeDataType::Float3);//Normals
            quadData.bufferData[0].layout.Add(AttributeDataType::Float2);//TexCoords
            return quadData;
        }
        static MeshData Cube()
        {
            MeshData cubeData{};
            cubeData.bufferData[0].data = {
                // Position               // Normal           // TexCoord

                // Front (+Z)
                -0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // 0
                 0.5f, -0.5f,  0.5f,       0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // 1
                 0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // 2
                -0.5f,  0.5f,  0.5f,       0.0f,  0.0f,  1.0f,   0.0f, 1.0f, // 3

                // Back (-Z)
                -0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // 4
                 0.5f, -0.5f, -0.5f,       0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // 5
                 0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,   0.0f, 1.0f, // 6
                -0.5f,  0.5f, -0.5f,       0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // 7

                // Right (+X)
                 0.5f, -0.5f,  0.5f,       1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // 8
                 0.5f, -0.5f, -0.5f,       1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // 9
                 0.5f,  0.5f, -0.5f,       1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // 10
                 0.5f,  0.5f,  0.5f,       1.0f,  0.0f, 0.0f,   0.0f, 1.0f, // 11

                 // Left (-X)
                 -0.5f, -0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // 12
                 -0.5f, -0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // 13
                 -0.5f,  0.5f,  0.5f,      -1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // 14
                 -0.5f,  0.5f, -0.5f,      -1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // 15

                 // Top (+Y)
                 -0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,   0.0f, 0.0f, // 16
                  0.5f,  0.5f,  0.5f,       0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // 17
                  0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,   1.0f, 1.0f, // 18
                 -0.5f,  0.5f, -0.5f,       0.0f,  1.0f,  0.0f,   0.0f, 1.0f, // 19

                 // Bottom (-Y)
                 -0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // 20
                  0.5f, -0.5f, -0.5f,       0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // 21
                  0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // 22
                 -0.5f, -0.5f,  0.5f,       0.0f, -1.0f,  0.0f,   0.0f, 1.0f  // 23
            };

            cubeData.bufferData[0].layout.Add(AttributeDataType::Float3); // Position
            cubeData.bufferData[0].layout.Add(AttributeDataType::Float3); // Normals
            cubeData.bufferData[0].layout.Add(AttributeDataType::Float2); // TexCoords

            cubeData.indices = {
                // Front
                 0,  1,  2,
                 2,  3,  0,

                 // Back
                  4,  5,  6,
                  6,  7,  4,

                  // Right
                   8,  9, 10,
                  10, 11,  8,

                  // Left
                  12, 13, 14,
                  14, 15, 12,

                  // Top
                  16, 17, 18,
                  18, 19, 16,

                  // Bottom
                  20, 21, 22,
                  22, 23, 20
            };
            cubeData.subMeshes.push_back(SubMesh(6, 0));
            cubeData.subMeshes.push_back(SubMesh(6, 6));
            cubeData.subMeshes.push_back(SubMesh(6, 12));
            cubeData.subMeshes.push_back(SubMesh(6, 18));
            cubeData.subMeshes.push_back(SubMesh(6, 24));
            cubeData.subMeshes.push_back(SubMesh(6, 30));

            return cubeData;
        }
    };
}
#pragma once
#include "Render/Mesh.h"
class Primitives {
public:
	static MeshData Triangle() {
		MeshData triangleData{};
		triangleData.bufferData.emplace_back();
		triangleData.bufferData[0].data = {
			// Position              // Color
			-0.8f, -0.8f, 0.0f,      1.0f, 0.0f, 0.0f, // Red
			 0.8f, -0.8f, 0.0f,      0.0f, 1.0f, 0.0f, // Green
			 0.0f,  0.8f, 0.0f,      0.0f, 0.0f, 1.0f  // Blue
		};
		triangleData.bufferData[0].layout.Add(AttributeDataType::Float3);
		triangleData.bufferData[0].layout.Add(AttributeDataType::Float3);
		return triangleData;
	}
	static MeshData Square() {
		MeshData squaredata{};
		squaredata.bufferData.emplace_back();
		squaredata.bufferData[0].data = {
			// Position 
			-0.8f, -0.8f, 0.0f,
			 0.8f, -0.8f, 0.0f,
			 -0.8f,  0.8f, 0.0f,
			 0.8f, 0.8f, 0.0f
		};
		squaredata.indices = {
			0,1,2,
			1,3,2
		};
		squaredata.bufferData[0].layout.Add(AttributeDataType::Float3);
		return squaredata;
	} 
    static MeshData Cube()
    {
        MeshData cubeData{};
        cubeData.bufferData.emplace_back();

        cubeData.bufferData[0].data = {
            // Position               // Color

            // Front - Red
            -0.5f, -0.5f,  0.5f,      1.0f, 0.0f, 0.0f, // 0
             0.5f, -0.5f,  0.5f,      1.0f, 0.0f, 0.0f, // 1
             0.5f,  0.5f,  0.5f,      1.0f, 0.0f, 0.0f, // 2
            -0.5f,  0.5f,  0.5f,      1.0f, 0.0f, 0.0f, // 3

            // Back - Green
            -0.5f, -0.5f, -0.5f,      0.0f, 1.0f, 0.0f, // 4
             0.5f, -0.5f, -0.5f,      0.0f, 1.0f, 0.0f, // 5
             0.5f,  0.5f, -0.5f,      0.0f, 1.0f, 0.0f, // 6
            -0.5f,  0.5f, -0.5f,      0.0f, 1.0f, 0.0f, // 7

            // Right - Blue
             0.5f, -0.5f,  0.5f,      0.0f, 0.0f, 1.0f, // 8
             0.5f, -0.5f, -0.5f,      0.0f, 0.0f, 1.0f, // 9
             0.5f,  0.5f, -0.5f,      0.0f, 0.0f, 1.0f, // 10
             0.5f,  0.5f,  0.5f,      0.0f, 0.0f, 1.0f, // 11

             // Left - Yellow
             -0.5f, -0.5f, -0.5f,      1.0f, 1.0f, 0.0f, // 12
             -0.5f, -0.5f,  0.5f,      1.0f, 1.0f, 0.0f, // 13
             -0.5f,  0.5f,  0.5f,      1.0f, 1.0f, 0.0f, // 14
             -0.5f,  0.5f, -0.5f,      1.0f, 1.0f, 0.0f, // 15

             // Top - Magenta
             -0.5f,  0.5f,  0.5f,      1.0f, 0.0f, 1.0f, // 16
              0.5f,  0.5f,  0.5f,      1.0f, 0.0f, 1.0f, // 17
              0.5f,  0.5f, -0.5f,      1.0f, 0.0f, 1.0f, // 18
             -0.5f,  0.5f, -0.5f,      1.0f, 0.0f, 1.0f, // 19

             // Bottom - Cyan
             -0.5f, -0.5f, -0.5f,      0.0f, 1.0f, 1.0f, // 20
              0.5f, -0.5f, -0.5f,      0.0f, 1.0f, 1.0f, // 21
              0.5f, -0.5f,  0.5f,      0.0f, 1.0f, 1.0f, // 22
             -0.5f, -0.5f,  0.5f,      0.0f, 1.0f, 1.0f  // 23
        };

        cubeData.bufferData[0].layout.Add(AttributeDataType::Float3); // Position
        cubeData.bufferData[0].layout.Add(AttributeDataType::Float3); // Color

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
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
};
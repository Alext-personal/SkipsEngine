#pragma once
#include <vector>
#include "Render/VertexArray.h"
struct SubMesh {
	uint32_t indexCount;
	uint32_t indexOffset;
	SubMesh(uint32_t count,uint32_t offset):indexCount(count),indexOffset(offset){}
};
struct MeshData { // for loading meshes
	struct VertexBufferData {
		std::vector<float> data{};
		VertexLayout layout{};
	};
	std::vector<VertexBufferData> bufferData{};
	std::vector<uint32_t> indices{};
	std::vector<SubMesh> subMeshes{}; //separate by material
};
class Mesh {
public:
	Mesh(const MeshData& data);
	const VertexArray& GetVAO() const { return m_vao; }
	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetIndexCount() const { return m_indexCount; }
	bool HasEBO() const {return m_ebo != nullptr; }
	const std::vector<SubMesh>& GetSubMeshes() const { return m_subMeshes; }
	~Mesh() = default;
private:
	uint32_t m_vertexCount{};
	uint32_t m_indexCount{};
	std::vector<std::unique_ptr<VertexBuffer>>m_vbos;
	std::unique_ptr<ElementBuffer> m_ebo;
	VertexArray m_vao;
	std::vector<SubMesh> m_subMeshes;

};
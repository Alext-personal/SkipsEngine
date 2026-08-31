#include "pch.h"
#include "Render/Mesh.h"
namespace Gaze {
	Mesh::Mesh(const MeshData& meshdata) :m_subMeshes(meshdata.subMeshes) {
		m_indexCount = meshdata.indices.size();
		uint32_t index{};
		for (const auto& vertexbuffer : meshdata.bufferData) {
			LOG_INFO("buffer data size : ${}  Buffer layout stride : ${}", vertexbuffer.data.size(), vertexbuffer.layout.GetStride());
			if (vertexbuffer.layout.GetStride() != 0)
				m_vertexCount += vertexbuffer.data.size() * sizeof(float) / vertexbuffer.layout.GetStride();
			m_vbos.push_back(std::make_unique<VertexBuffer>(vertexbuffer.data));
			m_vao.AddVertexBuffer(*m_vbos[index++], vertexbuffer.layout);
		}
		if (!meshdata.indices.empty())
		{
			m_ebo = std::make_unique<ElementBuffer>(meshdata.indices);
			m_vao.SetElementBuffer(*m_ebo);
			LOG_INFO("DID ELEMENT BUFFER SHIT");
		}
	}
}
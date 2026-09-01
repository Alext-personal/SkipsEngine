#include "pch.h"
#include "Core/Layer.h"
namespace Gaze {
	Layer::Layer(const std::string& layername) {
		m_name = layername;
	}
	Layer::~Layer() {}
}
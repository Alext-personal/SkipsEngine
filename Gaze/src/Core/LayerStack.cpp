#include "pch.h"
#include "Core/LayerStack.h"
namespace Gaze {
	LayerStack::LayerStack() {
		m_lastLayer = m_layers.begin();
	}
	LayerStack::~LayerStack() {
		for (Layer* layer : m_layers) {
			layer->OnDetach();
			delete layer;
		}
	}
	void LayerStack::PushLayer(Layer* layer) {
		m_lastLayer = m_layers.emplace(m_lastLayer, layer);
		layer->OnAttach();
	}
	void LayerStack::PushOverlay(Layer* overlay) {
		m_layers.emplace_back(overlay);
		overlay->OnAttach();
	}
	void LayerStack::PopLayer(Layer* layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			(*it)->OnDetach();
			m_layers.erase(it);
			m_lastLayer--;
		}
	}
	void LayerStack::PopOverlay(Layer* overlay) {
		auto it = std::find(m_layers.begin(), m_layers.end(), overlay);
		if (it != m_layers.end()) {
			(*it)->OnDetach();
			m_layers.erase(it);
		}
	}
}
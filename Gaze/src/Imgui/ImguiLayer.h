#pragma once
#include "Core/Layer.h"
#include "Scene/Component.h"
namespace Gaze {
	class ImguiLayer : public Layer {
	public:
		ImguiLayer();
		~ImguiLayer();
		void OnUpdate(float dt) override;
		void OnEvent(Event& e) override;
		void OnAttach() override;
		void OnDetach() override;
		void OnImguiRender() override;

		void Begin();
		void End();

		float rot[3]{0,0,0}; // test
		float scl[3]{1,1,1}; // test
		float pos[3]{0,0,0}; //test
		Transform testTransform; // test
	}; 
}
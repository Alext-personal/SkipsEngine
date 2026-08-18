#include "Core/Application.h"
int main() {
	std::unique_ptr<Gaze::Application> app = std::make_unique<Gaze::Application>();
	app->Run();
}
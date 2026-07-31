#include "Window.h"
int main() {
	Window window(640, 480,"SkipsEngine");
	while (!window.ShouldClose()) {
		glClear(GL_COLOR_BUFFER_BIT);
		window.SwapBuffers();
		window.PollEvents();
	}
}
#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <cstdint>
#include <filesystem>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Core/Helpers.h"
#include "Core/Log.h"
#include "Core/UUID.h"

#include "Events/Event.h"
#include "Events/InputEvents.h"
#include "Events/WindowEvents.h"
#include "Input/KeyCodes.h"
#include "Input/MouseCodes.h"

#ifdef _WIN32
#include <Windows.h>
#endif
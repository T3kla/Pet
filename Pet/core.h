#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include <iostream>

#include "math.h"
#include "var.h"

#include "engine.h"
#include "input.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

typedef std::function<void()> Job;

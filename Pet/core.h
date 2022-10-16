#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <stdint.h>
#include <vector>

using i16 = int16_t;               //
using i32 = int32_t;               //
using i64 = int64_t;               //
using u16 = uint16_t;              //
using u32 = uint32_t;              //
using u64 = uint64_t;              //
using job = std::function<void()>; //
template <typename... Ts>          //
using list = std::vector<Ts...>;   //

#include "engine.h"
#include "input.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

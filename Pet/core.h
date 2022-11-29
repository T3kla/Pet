#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>

#include <functional>
#include <iostream>
#include <optional>
#include <stdint.h>
#include <string.h>

#include <array>
#include <vector>

#include <set>
#include <unordered_set>

#include <map>
#include <unordered_map>

using i16 = int16_t;               //
using i32 = int32_t;               //
using i64 = int64_t;               //
using u16 = uint16_t;              //
using u32 = uint32_t;              //
using u64 = uint64_t;              //
using job = std::function<void()>; //
using str = std::string;           //

template <typename... Ts>                   //
using opt = std::optional<Ts...>;           //
template <typename... Ts>                   //
using array = std::array<Ts...>;            //
template <typename... Ts>                   //
using list = std::vector<Ts...>;            //
template <typename... Ts>                   //
using oset = std::set<Ts...>;               //
template <typename... Ts>                   //
using set = std::unordered_set<Ts...>;      //
template <typename... Ts>                   //
using odic = std::map<Ts...>;               //
template <typename... Ts>                   //
using dic = std::unordered_map<Ts...>;      //
template <typename... Ts>                   //
using omap = std::multimap<Ts...>;          //
template <typename... Ts>                   //
using map = std::unordered_multimap<Ts...>; //

#ifdef _DEBUG
static bool PetDebug = false;
#else
static bool PetDebug = false;
#endif

#include "engine.h"
#include "input.h"
#include "logger.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>

#include <fstream>
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

#include <limits>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "logger.h"

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using job = std::function<void()>;
using str = std::string;

template <typename... Ts> using opt = std::optional<Ts...>;
template <typename Ts, size_t Ta> using arr = std::array<Ts, Ta>;
template <typename... Ts> using list = std::vector<Ts...>;
template <typename... Ts> using oset = std::set<Ts...>;
template <typename... Ts> using set = std::unordered_set<Ts...>;
template <typename... Ts> using odic = std::map<Ts...>;
template <typename... Ts> using dic = std::unordered_map<Ts...>;
template <typename... Ts> using omap = std::multimap<Ts...>;
template <typename... Ts> using map = std::unordered_multimap<Ts...>;
template <typename... Ts> using del = std::function<Ts...>;
template <typename... Ts> using limits = std::numeric_limits<Ts...>;

#ifdef _DEBUG
static bool IsDebug = true;
#else
static bool IsDebug = false;
#endif

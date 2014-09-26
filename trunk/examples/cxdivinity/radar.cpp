// Copyright (C) 2010-2014 Joshua Boyce.
// See the file COPYING for copying permission.

#include "radar.hpp"

#include <d3d11.h>

#include <algorithm>
#include <cstdint>
#include <fstream>

#include <hadesmem/config.hpp>
#include <hadesmem/detail/trace.hpp>
#include <hadesmem/error.hpp>

#include "root_window.hpp"

namespace
{
std::size_t g_on_frame_callback_id_d3d9{static_cast<std::uint32_t>(-1)};
std::size_t g_on_frame_callback_id_dxgi{static_cast<std::uint32_t>(-1)};

void UpdateRadar()
{
  auto const hwnd = GetWindowHandle();
  if (!hwnd || !GetRadarEnabled())
  {
    return;
  }

  static RadarData radar_data;
  radar_data.player_ = {{200, 100, 100}, 0};
  radar_data.units_.emplace_back(
    RadarData::UnitData{Vec3f{250, 150, 50}, RGB(255, 0, 0), "Cat"});
  radar_data.units_.emplace_back(
    RadarData::UnitData{Vec3f{200, 50, 150}, RGB(0, 255, 0), "Dog"});
  radar_data.units_.emplace_back(
    RadarData::UnitData{Vec3f{125, 0, 75}, RGB(0, 0, 255), "raptorfactor"});
  radar_data.units_.emplace_back(RadarData::UnitData{
    Vec3f{150, -50, 150}, RGB(255, 255, 0), "Blah Blah Blah"});
  radar_data.units_.emplace_back(
    RadarData::UnitData{Vec3f{175, -150, 25}, RGB(0, 255, 255), "Foo"});

  PostMessageW(
    hwnd, WM_APP_UPDATE_RADAR, reinterpret_cast<WPARAM>(&radar_data), 0);

  std::unique_lock<std::mutex> lock(GetRadarFrameMutex());
  auto& radar_frame_processed = GetRadarFrameProcessed();
  while (!radar_frame_processed)
  {
    GetRadarFrameConditionVariable().wait(lock);
  }

  radar_frame_processed = false;
}

void OnFrame()
{
  static std::uint64_t frames{};
  if (frames++ % 30)
  {
    UpdateRadar();
  }
}

void OnFrameD3D9(IDirect3DDevice9* /*device*/)
{
  return OnFrame();
}

void OnFrameDXGI(IDXGISwapChain* /*swap_chain*/)
{
  return OnFrame();
}
}

void InitializeRadar(hadesmem::cerberus::PluginInterface* cerberus)
{
  HADESMEM_DETAIL_TRACE_A("Initializing.");

  g_on_frame_callback_id_d3d9 =
    cerberus->GetD3D9Interface()->RegisterOnFrameCallback(OnFrameD3D9);

  g_on_frame_callback_id_dxgi =
    cerberus->GetDXGIInterface()->RegisterOnFrameCallback(OnFrameDXGI);
}

void CleanupRadar(hadesmem::cerberus::PluginInterface* cerberus)
{
  HADESMEM_DETAIL_TRACE_A("Cleaning up.");

  if (g_on_frame_callback_id_d3d9 != static_cast<std::uint32_t>(-1))
  {
    cerberus->GetD3D9Interface()->UnregisterOnFrameCallback(
      g_on_frame_callback_id_d3d9);
    g_on_frame_callback_id_d3d9 = static_cast<std::uint32_t>(-1);
  }

  if (g_on_frame_callback_id_dxgi != static_cast<std::uint32_t>(-1))
  {
    cerberus->GetDXGIInterface()->UnregisterOnFrameCallback(
      g_on_frame_callback_id_dxgi);
    g_on_frame_callback_id_dxgi = static_cast<std::uint32_t>(-1);
  }
}

std::mutex& GetRadarFrameMutex()
{
  static std::mutex radar_frame_mutex;
  return radar_frame_mutex;
}

std::condition_variable& GetRadarFrameConditionVariable()
{
  static std::condition_variable radar_frame_condition_variable;
  return radar_frame_condition_variable;
}

bool& GetRadarFrameProcessed()
{
  static bool radar_frame_processed = false;
  return radar_frame_processed;
}

bool& GetRadarEnabled()
{
  static bool radar_frame_enabled = true;
  return radar_frame_enabled;
}

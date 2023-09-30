// Released under the MIT License. See LICENSE for details.

#ifndef BALLISTICA_BASE_APP_ADAPTER_APP_ADAPTER_SDL_H_
#define BALLISTICA_BASE_APP_ADAPTER_APP_ADAPTER_SDL_H_

#include "ballistica/base/base.h"
#if BA_SDL_BUILD

#include <vector>

#include "ballistica/base/app_adapter/app_adapter.h"
#include "ballistica/shared/math/vector2f.h"

// Predeclare for pointers.
struct SDL_Window;

namespace ballistica::base {

class AppAdapterSDL : public AppAdapter {
 public:
  /// Return g_base->app_adapter as an AppAdapterSDL. (Assumes it actually
  /// is one).
  static AppAdapterSDL* Get() {
    assert(g_base && g_base->app_adapter != nullptr);
    assert(dynamic_cast<AppAdapterSDL*>(g_base->app_adapter)
           == static_cast<AppAdapterSDL*>(g_base->app_adapter));
    return static_cast<AppAdapterSDL*>(g_base->app_adapter);
  }

  AppAdapterSDL();

  void OnMainThreadStartApp() override;
  void DoApplyAppConfig() override;

  auto CanToggleFullscreen() -> bool const override;
  auto SupportsVSync() -> bool const override;
  auto SupportsMaxFPS() -> bool const override;

 protected:
  void DoPushMainThreadRunnable(Runnable* runnable) override;
  void RunMainThreadEventLoopToCompletion() override;
  void DoExitMainThreadEventLoop() override;

 private:
  void SetScreen_(bool fullscreen, int max_fps, VSyncRequest vsync_requested,
                  TextureQualityRequest texture_quality_requested,
                  GraphicsQualityRequest graphics_quality_requested);
  void HandleSDLEvent_(const SDL_Event& event);
  void UpdateScreenSizes_();
  void ReloadRenderer_(bool fullscreen,
                       GraphicsQualityRequest graphics_quality_requested,
                       TextureQualityRequest texture_quality_requested);
  void OnSDLJoystickAdded_(int index);
  void OnSDLJoystickRemoved_(int index);
  // Given an SDL joystick ID, returns our Ballistica input for it.
  auto GetSDLJoystickInput_(int sdl_joystick_id) const -> JoystickInput*;
  // The same but using sdl events.
  auto GetSDLJoystickInput_(const SDL_Event* e) const -> JoystickInput*;
  // void DoSwap_();
  // void SwapBuffers_();
  // void UpdateAutoVSync_(int diff);
  void AddSDLInputDevice_(JoystickInput* input, int index);
  void RemoveSDLInputDevice_(int index);
  void SleepUntilNextEventCycle_(microsecs_t cycle_start_time);
  // millisecs_t last_swap_time_{};
  // millisecs_t swap_start_time_{};
  // int too_slow_frame_count_{};
  // bool auto_vsync_{};
  // bool vsync_enabled_{true};
  // float average_vsync_fps_{60.0f};
  // int vsync_good_frame_count_{};
  // int vsync_bad_frame_count_{};
  uint32_t sdl_runnable_event_id_{};
  std::vector<JoystickInput*> sdl_joysticks_;
  /// This is in points, not pixels.
  Vector2f window_size_{1.0f, 1.0f};
  SDL_Window* sdl_window_{};
  void* sdl_gl_context_{};
  // SDL_Surface* sdl_screen_surface_{};
  bool done_{};
  bool fullscreen_{};
  VSync vsync_{VSync::kUnset};
  bool vsync_actually_enabled_{};
  microsecs_t oversleep_{};
  int max_fps_{60};
  bool debug_log_sdl_frame_timing_{};
  // std::unique_ptr<GLContext> gl_context_;

  // TEMP
  // friend class GLContext;
  friend class GraphicsServer;
};

}  // namespace ballistica::base

#endif  // BA_SDL_BUILD

#endif  // BALLISTICA_BASE_APP_ADAPTER_APP_ADAPTER_SDL_H_

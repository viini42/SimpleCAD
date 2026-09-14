#pragma once

#include "app/app_state.hpp"
#include "command/pending_command.hpp"
#include "core/vec2.hpp"
#include "render/camera.hpp"
#include "scene/scene.hpp"
#include "ui/command_console.hpp"

#include <functional>
#include <string>
#include <vector>

namespace simple_cad
{
  // Parses command-line text (typed into the CommandConsole) and drives multi-step,
  // AutoCAD-style commands that need one or more points supplied either by typing
  // "x y" / "x,y" or by clicking on the canvas (see SubmitPoint).
  class CommandInterpreter
  {
  public:
    CommandInterpreter(Scene& scene,
                       Camera& camera,
                       AppState& state,
                       CommandConsole& console,
                       std::function<void()> on_quit);

    // Runs one full line of text submitted from the console.
    void Execute(const std::string& line);

    // Feeds a point picked with the mouse (already snapped, in world coordinates) into
    // whichever command is currently waiting for one. No-op when nothing is pending.
    void SubmitPoint(Vec2 world_point);

    // Aborts the in-progress command, if any.
    void Cancel();

    [[nodiscard]] bool HasPendingPoint() const { return m_pending != PendingCommand::None; }

    [[nodiscard]] PendingCommand Pending() const { return m_pending; }

    [[nodiscard]] const std::vector<Vec2>& CollectedPoints() const { return m_collected_points; }

    [[nodiscard]] const std::string& Prompt() const { return m_prompt; }

  private:
    void Dispatch(const std::vector<std::string>& tokens);
    void HandlePendingInput(const std::vector<std::string>& tokens);

    void CmdPoint(const std::vector<std::string>& args);
    void CmdLine(const std::vector<std::string>& args);
    void CmdCircle(const std::vector<std::string>& args);
    void CmdRect(const std::vector<std::string>& args);
    void CmdPolyline(const std::vector<std::string>& args);
    void CmdColor(const std::vector<std::string>& args);
    void CmdGrid(const std::vector<std::string>& args);
    void CmdSnap(const std::vector<std::string>& args);
    void CmdObjectSnap(const std::vector<std::string>& args);
    void CmdZoom(const std::vector<std::string>& args);
    void CmdImport(const std::vector<std::string>& args);
    void CmdClear();
    void CmdUndo();
    void CmdList();
    void CmdHelp();

    void CompleteLine();
    void CompleteRect();
    void CompleteCircleWithPoint(Vec2 point);
    void CompleteCircleWithRadius(double radius);
    void CompletePolyline(bool close);
    void UndoLastPolylinePoint();

    void ResetPending();
    void SetPromptAndLog(std::string text);
    void Log(std::string_view text);
    void LogError(std::string_view text);

    Scene& m_scene;
    Camera& m_camera;
    AppState& m_state;
    CommandConsole& m_console;
    std::function<void()> m_on_quit;

    PendingCommand m_pending{ PendingCommand::None };
    std::vector<Vec2> m_collected_points;
    std::string m_prompt;
  };
} // namespace simple_cad

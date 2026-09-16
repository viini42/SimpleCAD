#include "command/command_interpreter.hpp"

#include "command/command_tokenizer.hpp"
#include "core/text_utils.hpp"
#include "io/model_reader.hpp"
#include "io/model_writer.hpp"
#include "io/xy_importer.hpp"

#include <optional>
#include <type_traits>
#include <variant>

namespace
{
  using simple_cad::ParseNumber;
  using simple_cad::ParsePoint;
  using simple_cad::Vec2;

  // Parses as many leading "x y" points from `args` as possible, up to `max_points`.
  std::vector<Vec2> ParseLeadingPoints(const std::vector<std::string>& args, std::size_t max_points)
  {
    std::vector<Vec2> points;
    std::size_t index = 0;
    while (points.size() < max_points)
    {
      const auto point = ParsePoint(args, index);
      if (!point)
        break;

      points.push_back(*point);
    }
    return points;
  }

  constexpr std::string_view POLYLINE_NEXT_PROMPT =
    "POLYLINE: specify next point (or 'done'/'close' to finish, 'undo' to remove last).";
  constexpr std::string_view POLYLINE_FIRST_PROMPT =
    "POLYLINE: specify first point (click or type x,y).";

  // Reassembles a file path split into whitespace-separated tokens. Loses runs of repeated
  // whitespace and any literal comma, since Tokenize() already discarded those.
  std::string JoinWithSpaces(const std::vector<std::string>& tokens)
  {
    std::string result;
    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
      if (i > 0)
        result += ' ';
      result += tokens[i];
    }
    return result;
  }
} // namespace

simple_cad::CommandInterpreter::CommandInterpreter(Scene& scene,
                                                   Camera& camera,
                                                   AppState& state,
                                                   CommandConsole& console,
                                                   std::function<void()> on_quit) :
    m_scene(scene),
    m_camera(camera),
    m_state(state),
    m_console(console),
    m_on_quit(std::move(on_quit))
{
}

void simple_cad::CommandInterpreter::Log(std::string_view text)
{
  m_console.PushLog(text);
}

void simple_cad::CommandInterpreter::LogError(std::string_view text)
{
  m_console.PushLog(std::string("! ") + std::string(text));
}

void simple_cad::CommandInterpreter::SetPromptAndLog(std::string text)
{
  m_prompt = text;
  Log(m_prompt);
}

void simple_cad::CommandInterpreter::ResetPending()
{
  m_pending = PendingCommand::None;
  m_collected_points.clear();
  m_prompt.clear();
}

void simple_cad::CommandInterpreter::Execute(const std::string& line)
{
  const std::vector<std::string> tokens = Tokenize(line);
  if (tokens.empty())
    return;

  if (m_pending != PendingCommand::None)
  {
    HandlePendingInput(tokens);
    return;
  }

  Dispatch(tokens);
}

void simple_cad::CommandInterpreter::HandlePendingInput(const std::vector<std::string>& tokens)
{
  const std::string first = ToLower(tokens[0]);
  if (first == "cancel" || first == "esc")
  {
    Cancel();
    return;
  }

  if (m_pending == PendingCommand::Polyline && tokens.size() == 1)
  {
    if (first == "done" || first == "finish")
    {
      CompletePolyline(false);
      return;
    }
    if (first == "close")
    {
      CompletePolyline(true);
      return;
    }
    if (first == "undo")
    {
      UndoLastPolylinePoint();
      return;
    }
  }

  const bool radius_allowed = m_pending == PendingCommand::Circle && m_collected_points.size() == 1;
  if (radius_allowed && tokens.size() == 1)
  {
    if (const auto radius = ParseNumber(tokens[0]))
    {
      CompleteCircleWithRadius(*radius);
      return;
    }
  }

  std::size_t index = 0;
  const auto point = ParsePoint(tokens, index);
  if (!point || index != tokens.size())
  {
    LogError(std::string("Expected a point (x y)") + (radius_allowed ? ", a radius" : "") +
             ", or 'cancel'.");
    return;
  }

  SubmitPoint(*point);
}

void simple_cad::CommandInterpreter::SubmitPoint(Vec2 world_point)
{
  switch (m_pending)
  {
  case PendingCommand::None:
    break;

  case PendingCommand::Point:
    m_scene.AddPoint(world_point, m_state.current_color);
    Log("Point added.");
    ResetPending();
    break;

  case PendingCommand::Line:
    m_collected_points.push_back(world_point);
    if (m_collected_points.size() == 1)
      SetPromptAndLog("LINE: specify second point.");
    else
      CompleteLine();
    break;

  case PendingCommand::Circle:
    if (m_collected_points.empty())
    {
      m_collected_points.push_back(world_point);
      SetPromptAndLog("CIRCLE: specify radius (click a point or type a number).");
    }
    else
    {
      CompleteCircleWithPoint(world_point);
    }
    break;

  case PendingCommand::Rect:
    m_collected_points.push_back(world_point);
    if (m_collected_points.size() == 1)
      SetPromptAndLog("RECT: specify opposite corner.");
    else
      CompleteRect();
    break;

  case PendingCommand::Polyline:
    m_collected_points.push_back(world_point);
    SetPromptAndLog(std::string(POLYLINE_NEXT_PROMPT));
    break;
  }
}

void simple_cad::CommandInterpreter::Cancel()
{
  if (m_pending == PendingCommand::None)
    return;

  Log("Command cancelled.");
  ResetPending();
}

void simple_cad::CommandInterpreter::CompleteLine()
{
  m_scene.AddLine(m_collected_points[0], m_collected_points[1], m_state.current_color);
  Log("Line added.");
  ResetPending();
}

void simple_cad::CommandInterpreter::CompleteRect()
{
  m_scene.AddRect(m_collected_points[0], m_collected_points[1], m_state.current_color);
  Log("Rectangle added.");
  ResetPending();
}

void simple_cad::CommandInterpreter::CompleteCircleWithPoint(Vec2 point)
{
  const double radius = (point - m_collected_points[0]).Length();
  if (radius <= 0.0)
  {
    LogError("Radius must be greater than zero; pick a different point.");
    return;
  }

  m_scene.AddCircle(m_collected_points[0], radius, m_state.current_color);
  Log("Circle added.");
  ResetPending();
}

void simple_cad::CommandInterpreter::CompleteCircleWithRadius(double radius)
{
  if (radius <= 0.0)
  {
    LogError("Radius must be greater than zero.");
    return;
  }

  m_scene.AddCircle(m_collected_points[0], radius, m_state.current_color);
  Log("Circle added.");
  ResetPending();
}

void simple_cad::CommandInterpreter::CompletePolyline(bool close)
{
  if (m_collected_points.size() < 2)
  {
    LogError("POLYLINE: need at least 2 points before finishing.");
    return;
  }

  std::vector<Vec2> points = m_collected_points;
  if (close)
    points.push_back(points.front());

  m_scene.AddPolyline(std::move(points), m_state.current_color);
  Log(close ? "Polyline added (closed)." : "Polyline added.");
  ResetPending();
}

void simple_cad::CommandInterpreter::UndoLastPolylinePoint()
{
  if (m_collected_points.empty())
  {
    LogError("POLYLINE: no points to undo yet.");
    return;
  }

  m_collected_points.pop_back();
  SetPromptAndLog(
    std::string(m_collected_points.empty() ? POLYLINE_FIRST_PROMPT : POLYLINE_NEXT_PROMPT));
}

void simple_cad::CommandInterpreter::Dispatch(const std::vector<std::string>& tokens)
{
  const std::string command = ToLower(tokens[0]);
  const std::vector<std::string> args(tokens.begin() + 1, tokens.end());

  if (command == "point" || command == "pt")
    CmdPoint(args);
  else if (command == "line" || command == "ln")
    CmdLine(args);
  else if (command == "circle" || command == "cir")
    CmdCircle(args);
  else if (command == "rect" || command == "rectangle")
    CmdRect(args);
  else if (command == "polyline" || command == "pline" || command == "pl")
    CmdPolyline(args);
  else if (command == "color" || command == "colour")
    CmdColor(args);
  else if (command == "grid")
    CmdGrid(args);
  else if (command == "snap")
    CmdSnap(args);
  else if (command == "osnap")
    CmdObjectSnap(args);
  else if (command == "zoom")
    CmdZoom(args);
  else if (command == "import")
    CmdImport(args);
  else if (command == "save")
    CmdSave(args);
  else if (command == "open" || command == "load")
    CmdOpen(args);
  else if (command == "clear")
    CmdClear();
  else if (command == "undo")
    CmdUndo();
  else if (command == "list")
    CmdList();
  else if (command == "help" || command == "?")
    CmdHelp();
  else if (command == "quit" || command == "exit")
  {
    if (m_on_quit)
      m_on_quit();
  }
  else
  {
    LogError("Unknown command '" + tokens[0] + "'. Type 'help' for a list of commands.");
  }
}

void simple_cad::CommandInterpreter::CmdPoint(const std::vector<std::string>& args)
{
  std::size_t index = 0;
  const auto point = ParsePoint(args, index);
  if (point)
  {
    if (index != args.size())
    {
      LogError("POINT: unexpected extra arguments.");
      return;
    }
    m_scene.AddPoint(*point, m_state.current_color);
    Log("Point added.");
    return;
  }

  if (!args.empty())
  {
    LogError("POINT: expected 'x y'.");
    return;
  }

  m_pending = PendingCommand::Point;
  SetPromptAndLog("POINT: specify location (click or type x,y).");
}

void simple_cad::CommandInterpreter::CmdLine(const std::vector<std::string>& args)
{
  const std::vector<Vec2> points = ParseLeadingPoints(args, 2);
  if (points.size() * 2 != args.size())
  {
    LogError("LINE: expected 'x1 y1 x2 y2'.");
    return;
  }

  m_pending = PendingCommand::Line;
  m_collected_points = points;

  if (m_collected_points.size() == 2)
  {
    CompleteLine();
    return;
  }

  SetPromptAndLog(m_collected_points.empty() ? "LINE: specify first point (click or type x,y)."
                                             : "LINE: specify second point.");
}

void simple_cad::CommandInterpreter::CmdCircle(const std::vector<std::string>& args)
{
  std::size_t index = 0;
  const auto center = ParsePoint(args, index);
  if (!center)
  {
    if (!args.empty())
    {
      LogError("CIRCLE: expected 'cx cy [radius]'.");
      return;
    }
    m_pending = PendingCommand::Circle;
    SetPromptAndLog("CIRCLE: specify center point (click or type x,y).");
    return;
  }

  m_collected_points = { *center };

  if (index == args.size())
  {
    m_pending = PendingCommand::Circle;
    SetPromptAndLog("CIRCLE: specify radius (click a point or type a number).");
    return;
  }

  const auto radius = ParseNumber(args[index]);
  if (!radius || index + 1 != args.size())
  {
    LogError("CIRCLE: expected a single numeric radius after the center point.");
    m_collected_points.clear();
    return;
  }

  CompleteCircleWithRadius(*radius);
}

void simple_cad::CommandInterpreter::CmdRect(const std::vector<std::string>& args)
{
  const std::vector<Vec2> points = ParseLeadingPoints(args, 2);
  if (points.size() * 2 != args.size())
  {
    LogError("RECT: expected 'x1 y1 x2 y2'.");
    return;
  }

  m_pending = PendingCommand::Rect;
  m_collected_points = points;

  if (m_collected_points.size() == 2)
  {
    CompleteRect();
    return;
  }

  SetPromptAndLog(m_collected_points.empty() ? "RECT: specify first corner (click or type x,y)."
                                             : "RECT: specify opposite corner.");
}

void simple_cad::CommandInterpreter::CmdPolyline(const std::vector<std::string>& args)
{
  const std::vector<Vec2> points = ParseLeadingPoints(args, args.size());
  if (points.size() * 2 != args.size())
  {
    LogError("POLYLINE: expected pairs of 'x y' coordinates.");
    return;
  }

  m_pending = PendingCommand::Polyline;
  m_collected_points = points;

  SetPromptAndLog(
    std::string(m_collected_points.empty() ? POLYLINE_FIRST_PROMPT : POLYLINE_NEXT_PROMPT));
}

void simple_cad::CommandInterpreter::CmdColor(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    Log("Current color: " + std::string(ColorName(m_state.current_color)));
    return;
  }

  if (ToLower(args[0]) == "list")
  {
    std::string names = "Available colors:";
    for (const NamedColor& entry : PALETTE)
      names += " " + std::string(entry.name);
    Log(names);
    return;
  }

  const auto color = ParseColor(args[0]);
  if (!color)
  {
    LogError("COLOR: unknown color '" + args[0] + "'. Try 'color list'.");
    return;
  }

  m_state.current_color = *color;
  Log("Color set to " + args[0] + ".");
}

void simple_cad::CommandInterpreter::CmdGrid(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("GRID: expected 'on', 'off' or 'size <n>'.");
    return;
  }

  const std::string option = ToLower(args[0]);
  if (option == "on")
  {
    m_state.grid_visible = true;
    Log("Grid on.");
    return;
  }

  if (option == "off")
  {
    m_state.grid_visible = false;
    Log("Grid off.");
    return;
  }

  if (option == "size")
  {
    const auto size = args.size() >= 2 ? ParseNumber(args[1]) : std::nullopt;
    if (!size || *size <= 0.0)
    {
      LogError("GRID SIZE: expected a positive number.");
      return;
    }
    m_state.grid_size = *size;
    Log("Grid size set to " + args[1] + ".");
    return;
  }

  LogError("GRID: unknown option '" + args[0] + "'.");
}

void simple_cad::CommandInterpreter::CmdSnap(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("SNAP: expected 'on' or 'off'.");
    return;
  }

  const std::string option = ToLower(args[0]);
  if (option == "on")
  {
    m_state.snap_enabled = true;
    Log("Snap on.");
    return;
  }

  if (option == "off")
  {
    m_state.snap_enabled = false;
    Log("Snap off.");
    return;
  }

  LogError("SNAP: unknown option '" + args[0] + "'.");
}

void simple_cad::CommandInterpreter::CmdObjectSnap(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("OSNAP: expected 'on' or 'off'.");
    return;
  }

  const std::string option = ToLower(args[0]);
  if (option == "on")
  {
    m_state.object_snap_enabled = true;
    Log("Object snap on.");
    return;
  }

  if (option == "off")
  {
    m_state.object_snap_enabled = false;
    Log("Object snap off.");
    return;
  }

  LogError("OSNAP: unknown option '" + args[0] + "'.");
}

void simple_cad::CommandInterpreter::CmdZoom(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("ZOOM: expected 'in', 'out', 'fit' or a factor.");
    return;
  }

  const std::string option = ToLower(args[0]);
  if (option == "fit" || option == "extents")
  {
    const auto bounds = m_scene.BoundingBox();
    if (!bounds)
    {
      m_camera.ResetView();
      Log("Nothing to fit; view reset.");
      return;
    }
    m_camera.Fit(*bounds);
    Log("Zoomed to fit.");
    return;
  }

  double factor = 1.0;
  if (option == "in")
    factor = 1.25;
  else if (option == "out")
    factor = 0.8;
  else if (const auto parsed = ParseNumber(option))
    factor = *parsed;
  else
  {
    LogError("ZOOM: unknown option '" + args[0] + "'.");
    return;
  }

  m_camera.ZoomAt(m_camera.ViewportSize() * 0.5, factor);
  Log("Zoom scale: " + std::to_string(m_camera.Scale()) + " px/unit.");
}

void simple_cad::CommandInterpreter::CmdImport(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("IMPORT: expected a file path.");
    return;
  }

  const std::string path = JoinWithSpaces(args);
  const auto result = ImportXyFile(m_scene, path, m_state.current_color);
  if (!result)
  {
    LogError("IMPORT: could not read '" + path + "' (missing file or malformed HED_XY content).");
    return;
  }

  Log("Imported " + std::to_string(result->points_imported) + " point(s) and " +
      std::to_string(result->edges_imported) + " edge(s) from '" + path + "'.");
  if (result->edges_skipped > 0)
    Log("Skipped " + std::to_string(result->edges_skipped) + " edge(s) with fewer than 2 points.");

  if (const auto bounds = m_scene.BoundingBox())
    m_camera.Fit(*bounds);
}

void simple_cad::CommandInterpreter::CmdSave(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("SAVE: expected a file path.");
    return;
  }

  const std::string path = JoinWithSpaces(args);
  if (!WriteModelFile(m_scene, path))
  {
    LogError("SAVE: could not write '" + path + "'.");
    return;
  }

  Log("Saved " + std::to_string(m_scene.Primitives().size()) + " primitive(s) to '" + path + "'.");
}

void simple_cad::CommandInterpreter::CmdOpen(const std::vector<std::string>& args)
{
  if (args.empty())
  {
    LogError("OPEN: expected a file path.");
    return;
  }

  const std::string path = JoinWithSpaces(args);
  auto loaded = ReadModelFile(path);
  if (!loaded)
  {
    LogError("OPEN: could not read '" + path + "' (missing file or invalid model content).");
    return;
  }

  m_scene = std::move(*loaded);
  Log("Opened '" + path + "' (" + std::to_string(m_scene.Primitives().size()) + " primitive(s)).");

  if (const auto bounds = m_scene.BoundingBox())
    m_camera.Fit(*bounds);
}

void simple_cad::CommandInterpreter::CmdClear()
{
  m_scene.Clear();
  Log("Scene cleared.");
}

void simple_cad::CommandInterpreter::CmdUndo()
{
  if (!m_scene.RemoveLast())
    LogError("Nothing to undo.");
  else
    Log("Last primitive removed.");
}

void simple_cad::CommandInterpreter::CmdList()
{
  const auto& primitives = m_scene.Primitives();
  if (primitives.empty())
  {
    Log("Scene is empty.");
    return;
  }

  for (const Primitive& primitive : primitives)
  {
    std::visit(
      [&](const auto& shape)
      {
        using ShapeType = std::decay_t<decltype(shape)>;
        std::string description = "#" + std::to_string(primitive.id) + " ";

        if constexpr (std::is_same_v<ShapeType, PointShape>)
          description += "point (" + std::to_string(shape.position.x) + ", " +
                         std::to_string(shape.position.y) + ")";
        else if constexpr (std::is_same_v<ShapeType, LineShape>)
          description += "line (" + std::to_string(shape.start.x) + ", " +
                         std::to_string(shape.start.y) + ") -> (" + std::to_string(shape.end.x) +
                         ", " + std::to_string(shape.end.y) + ")";
        else if constexpr (std::is_same_v<ShapeType, CircleShape>)
          description += "circle center (" + std::to_string(shape.center.x) + ", " +
                         std::to_string(shape.center.y) + ") r=" + std::to_string(shape.radius);
        else if constexpr (std::is_same_v<ShapeType, RectShape>)
          description += "rect (" + std::to_string(shape.corner_a.x) + ", " +
                         std::to_string(shape.corner_a.y) + ") -> (" +
                         std::to_string(shape.corner_b.x) + ", " +
                         std::to_string(shape.corner_b.y) + ")";
        else
          description += "polyline with " + std::to_string(shape.points.size()) +
                         " points, starting at (" + std::to_string(shape.points.front().x) + ", " +
                         std::to_string(shape.points.front().y) + ")";

        Log(description);
      },
      primitive.shape);
  }
}

void simple_cad::CommandInterpreter::CmdHelp()
{
  Log("Commands:");
  Log("  point x y            | pt                add a point");
  Log("  line [x1 y1 x2 y2]   | ln                add a line (click or type points)");
  Log("  circle [cx cy [r]]   | cir               add a circle");
  Log("  rect [x1 y1 x2 y2]                       add a rectangle");
  Log("  polyline [x1 y1 x2 y2 ...]  | pline, pl   add a multi-point line");
  Log("    while drawing: 'done' finish, 'close' finish+close, 'undo' remove last point");
  Log("  color <name|#hex>    | color list        set the draw color");
  Log("  grid on|off|size <n>                     grid visibility / spacing");
  Log("  snap on|off                              toggle grid snapping");
  Log("  osnap on|off                             toggle snapping to existing geometry");
  Log("  zoom in|out|fit|<factor>                 zoom the camera");
  Log("  import <path>                            run a HED_XY \"script\": adds points/polylines");
  Log("  save <path>                               save the scene to a model file (JSON)");
  Log("  open <path>          | load               replace the scene with a saved model file");
  Log("  list                                     list every primitive");
  Log("  undo                                     remove the last primitive");
  Log("  clear                                    remove every primitive");
  Log("  cancel                                   abort the current command");
  Log("  quit | exit                              close the application");
  Log("Shortcuts: F2 zoom fit, F7 toggle grid, F8 toggle snap, F9 toggle osnap, Escape cancel.");
}

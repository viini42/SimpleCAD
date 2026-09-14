#pragma once

namespace simple_cad
{
  // Which multi-step command (if any) is currently waiting for a point. Kept in its own
  // header, free of any other dependency, so both command/ and render/ can use it without
  // creating a dependency cycle between the two.
  enum class PendingCommand
  {
    None,
    Point,
    Line,
    Circle,
    Rect,
  };
} // namespace simple_cad

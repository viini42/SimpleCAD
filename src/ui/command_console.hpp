#pragma once

#include <deque>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace simple_cad
{
  // A single-line, always-focused text input with a scrolling log, similar to the
  // command line found in traditional CAD tools. Owns no rendering or SDL state.
  class CommandConsole
  {
  public:
    using SubmitHandler = std::function<void(const std::string&)>;

    explicit CommandConsole(SubmitHandler on_submit) : m_on_submit(std::move(on_submit)) {}

    void AppendText(std::string_view text);
    void Backspace();
    void Submit();
    void HistoryUp();
    void HistoryDown();
    void ClearInput();

    void PushLog(std::string_view line);

    [[nodiscard]] const std::string& InputBuffer() const { return m_input_buffer; }

    [[nodiscard]] const std::deque<std::string>& LogLines() const { return m_log; }

  private:
    static constexpr std::size_t MAX_LOG_LINES = 200;

    SubmitHandler m_on_submit;
    std::string m_input_buffer;
    std::vector<std::string> m_history;
    std::size_t m_history_cursor{ 0 };
    std::deque<std::string> m_log;
  };
} // namespace simple_cad

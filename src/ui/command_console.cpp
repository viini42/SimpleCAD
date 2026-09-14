#include "ui/command_console.hpp"

void simple_cad::CommandConsole::AppendText(std::string_view text)
{
  m_input_buffer.append(text);
}

void simple_cad::CommandConsole::Backspace()
{
  if (!m_input_buffer.empty())
    m_input_buffer.pop_back();
}

void simple_cad::CommandConsole::Submit()
{
  if (m_input_buffer.empty())
    return;

  PushLog("> " + m_input_buffer);
  m_history.push_back(m_input_buffer);
  m_history_cursor = m_history.size();

  const std::string command_line = m_input_buffer;
  m_input_buffer.clear();

  if (m_on_submit)
    m_on_submit(command_line);
}

void simple_cad::CommandConsole::HistoryUp()
{
  if (m_history.empty() || m_history_cursor == 0)
    return;

  --m_history_cursor;
  m_input_buffer = m_history[m_history_cursor];
}

void simple_cad::CommandConsole::HistoryDown()
{
  if (m_history_cursor >= m_history.size())
    return;

  ++m_history_cursor;
  m_input_buffer =
    m_history_cursor < m_history.size() ? m_history[m_history_cursor] : std::string{};
}

void simple_cad::CommandConsole::ClearInput()
{
  m_input_buffer.clear();
}

void simple_cad::CommandConsole::PushLog(std::string_view line)
{
  m_log.emplace_back(line);
  if (m_log.size() > MAX_LOG_LINES)
    m_log.pop_front();
}

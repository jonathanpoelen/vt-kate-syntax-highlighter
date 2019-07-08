/*
    Copyright (C) 2018 Jonathan Poelen <jonathan.poelen@gmail.com>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vttracehighlighter.h"
#include "theme_buffer.h"
// #include "vtsyntaxhighlighting_logging.h"

#include <KF5/KSyntaxHighlighting/definition.h>
#include <KF5/KSyntaxHighlighting/format.h>
#include <KF5/KSyntaxHighlighting/state.h>
#include <KF5/KSyntaxHighlighting/theme.h>

//#include <QDebug>
#include <QTextStream>
//#include <QLoggingCategory>

#include <charconv>


using namespace VtSyntaxHighlighting;
using namespace KSyntaxHighlighting;

namespace
{
  template<std::size_t N>
  QByteArray& setBuf(QByteArray& a, MiniBuf<N> const& buf)
  {
    return a.setRawData(buf.data(), buf.size());
  }
}

struct VtTraceHighlighting::Info
{
  int offset;
  int length;
  QString name;
  ThemeFormatBuf theme_buffer;
};

VtTraceHighlighting::VtTraceHighlighting() = default;
VtTraceHighlighting::~VtTraceHighlighting() = default;

void VtTraceHighlighting::enableTraceName(bool withTraceName)
{
  m_enableTraceName = withTraceName;
}

void VtTraceHighlighting::enableTraceRegion(bool withTraceRegion)
{
  m_enableTraceRegion = withTraceRegion;
}

namespace
{
  struct Line
  {
    QString s1;
    QString s2;
    int len1 = 0;
    int len2 = 0;

    template<class T1, class T2, class T3, class T4>
    void pushName(int offset, T1 const& stateStyle, T2 const& nameStyle, T3 const& name, T4 const& infoStyle)
    {
      assert(offset >= len2);
      int n = offset - len2;
      len2 += name.size() + n;
      _expandLine(s2, n);
      s2 += stateStyle;
      s2 += nameStyle;
      s2 += name;
      s2 += infoStyle;
    };

    template<class T1, class T2, class T3>
    void pushGraph(int offset, T1 const& stateStyle, T2 const& graph, T3 const& infoStyle)
    {
      assert(offset >= len1);
      int n1 = offset - len1;
      len1 += n1 + 1;
      _expandLine(s1, n1);
      int ps1 = s1.size();
      s1 += stateStyle;
      s1 += graph;
      s1 += infoStyle;
      if (offset >= len2) {
        int n2 = offset - len2;
        len2 += n2 + 1;
        _expandLine(s2, n2);
        s2 += s1.rightRef(s1.size() - ps1);
      }
    };

  private:
    static void _expandLine(QString& s, int n)
    {
      constexpr std::string_view spaces = "                                                   ";
      assert(n >= 0);
      for (; n > int(size(spaces)); n -= size(spaces)) {
        s += spaces.data();
      }
      s += spaces.data() + (spaces.size() - n);
    }
  };
}

void VtTraceHighlighting::highlight()
{
  initStyle();

  // TODO
  QString infoStyle = "\x1b[0m";
  // QString nameStyle = "";
  QString nameStyle = "\x1b[7;2m";
  // QString infoStyle = "\x1b[0;48;2;66;66;66m";
  QString graph = "\x1b[21;23;24;2m│";
  // QString sep = "\x1b[48;2;66;66;66m────┄┄··\x1b[K\x1b[0m\n";
  QString sep = "\x1b[48;2;66;66;66m────····\x1b[K\x1b[0m\n";

  std::vector<Line> lines;

  auto selectLine = [&](int offset) -> Line& {
    auto p = std::find_if(begin(lines), end(lines), [=](Line const& line) {
      return line.len2 < offset;
    });
    return (p == end(lines)) ? lines.emplace_back() : *p;
  };

  State state;
  while (!m_in->atEnd())
  {
    m_currentLine = m_in->readLine();
    state = highlightLine(m_currentLine, state);
    *m_out << '\n';

    lines.clear();
    for (Info const& info : m_infos)
    {
      Line& line = selectLine(info.offset);
      setBuf(m_buffer, info.theme_buffer);
      line.pushName(info.offset, m_buffer, nameStyle, info.name, infoStyle);

      for (Line* pline = lines.data(); pline <= &line; ++pline)
      {
        pline->pushGraph(info.offset, m_buffer, graph, infoStyle);
      }
    }

    *m_out << infoStyle;
    for (Line& line : lines)
    {
      *m_out << line.s1 << '\n' << line.s2 << '\n';
    }
    *m_out << sep;

    m_infos.clear();

    if (!m_isBuffered)
    {
      m_out->flush();
    }
  }
}

void VtTraceHighlighting::applyFormat(int offset, int length, const Format& format)
{
  auto&& current_theme = theme();
  bool isDefaultTextStyle = format.isDefaultTextStyle(current_theme);

  if (!isDefaultTextStyle)
  {
    auto buf = create_vt_theme_buffer(format, m_current_theme);
    *m_out << setBuf(m_buffer, buf);
    if (m_enableTraceName)
    {
      m_infos.push_back(Info{offset, length, format.name(), buf});
    }
  }
  else if (m_enableTraceName)
  {
    m_infos.push_back(Info{offset, length, format.name(), {}});
  }

  // if (m_enableTraceName)
  // {
  //   // inverse color name
  //   *m_out << "\x1b[7m" << format.name() << "\x1b[27m";
  // }

  // TODO show auto-completion

  *m_out << m_currentLine.mid(offset, length);

  if (!isDefaultTextStyle)
  {
    *m_out << m_defautStyle;
  }

  if (m_region.size())
  {
    // reverse color
    *m_out << "\x1b[7;3m" << m_region.c_str() << "\x1b[27;23m";
    m_region.clear();
  }
}

void VtTraceHighlighting::applyFolding(int offset, int length, FoldingRegion region)
{
  (void)offset;
  (void)length;
  if (m_enableTraceRegion)
  {
    std::array<char, 10> str;
    auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), region.id());
    (void)ec;
    auto len = p - str.data();
    str[len] = (region.type() == FoldingRegion::Begin) ? '(' : ')';
    m_region.append(str.data(), len + 1);
  }
}

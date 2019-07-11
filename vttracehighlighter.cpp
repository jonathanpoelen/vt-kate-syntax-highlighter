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
  const QString spaceLine = QStringLiteral(
    "                              "
    "                              "
    "                              "
  );
  const QString continuationLine = QStringLiteral(
    "------------------------------"
    "------------------------------"
    "------------------------------"
  );

  void expandLine(QString& s, int n, QString const& fill)
  {
    assert(n >= 0);
    for (; n > int(fill.size()); n -= fill.size()) {
      s += fill;
    }
    s += fill.left(n);
  }

  struct Line
  {
    QString s1;
    QString s2;
    int len1 = 0;
    int len2 = 0;
    int endNameLen = 0;

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
      endNameLen = len2;
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
      expandLine(s, n, spaceLine);
    }
  };
}

struct VtTraceHighlighting::InfoFormat
{
  int offset;
  QString name;
  QString style;
};

struct VtTraceHighlighting::InfoRegion
{
  int offset;
  int bindIndex;
  quint16 id;
  bool isEnd;
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

void VtTraceHighlighting::highlight()
{
  initStyle();

  // TODO
  // QString infoStyle = "\x1b[0m";
  QString infoStyle = "\x1b[0;48;2;34;34;34m";
  // QString nameStyle = "";
  QString nameStyle = "\x1b[7;2m";
  QString idStyle = "\x1b[7;3;2m";
  QString graph = "\x1b[21;23;24;2m|";
  QString CloseGraph = "\x1b[21;23;24;1m|";
  // QString sep = "\x1b[48;2;66;66;66m────┄┄··\x1b[K\x1b[0m\n";
  QString sep = "\x1b[48;2;66;66;66m────····\x1b[K\x1b[0m\n";

  std::vector<Line> lines;

  auto selectLine = [&](int offset) -> Line& {
    auto p = std::find_if(begin(lines), end(lines), [=](Line const& line) {
      return line.endNameLen < offset;
    });
    return (p == end(lines)) ? lines.emplace_back() : *p;
  };

  // TODO show auto-completion

  QString regionName;

  State state;
  while (!m_in->atEnd())
  {
    m_currentLine = m_in->readLine();
    m_currentFormatedLine.clear();
    state = highlightLine(m_currentLine, state);

    if (!m_regions.empty())
    {
      lines.clear();

      bool hasOnlyClose = false;
      for (InfoRegion& info : m_regions)
      {
        if (info.offset < 0)
        {
          regionName.setNum(info.id);
          int n = info.bindIndex - regionName.size();
          if (n > 0)
          {
            QString tmp = regionName;
            regionName.clear();
            expandLine(regionName, n, continuationLine);
            regionName += ')';
            regionName += tmp;
          }
          else
          {
            regionName.prepend(')');
          }
          Line& line = selectLine(0);
          line.pushName(0, QString(), idStyle, regionName, infoStyle);
          hasOnlyClose = true;
          info.bindIndex = &line - lines.data();
        }
      }

      if (hasOnlyClose)
      {
        for (Line& line : lines)
        {
          line.pushGraph(0, QString(), CloseGraph, infoStyle);
        }
      }

      for (InfoRegion& info : m_regions)
      {
        if (info.offset < 0)
        {
          continue;
        }

        Line * pline;
        QString* graphStyle;

        regionName.setNum(info.id);
        if (info.isEnd)
        {
          pline = &lines[m_regions[info.bindIndex].bindIndex];
          graphStyle = &CloseGraph;
        }
        else
        {
          regionName += '(';
          if (info.bindIndex >= 0) {
            int n = m_regions[info.bindIndex].offset - info.offset - regionName.size();
            if (n > 0)
            {
              expandLine(regionName, n, continuationLine);
            }
            regionName += ')';
          }
          pline = &selectLine(info.offset);
          pline->pushName(info.offset, QString(), idStyle, regionName, infoStyle);
          graphStyle = &graph;
          info.bindIndex = pline - lines.data();
        }

        for (Line* p = lines.data(); p <= pline; ++p)
        {
          p->pushGraph(info.offset, QString(), *graphStyle, infoStyle);
        }
      }

      *m_out << infoStyle;
      auto p = lines.rbegin();
      *m_out << p->s2 << "\x1b[K\n" << p->s1 << infoStyle;
      for (auto pend = lines.rend(); ++p != pend;)
      {
        *m_out << "\x1b[K\n" << p->s2 << "\x1b[K\n" << p->s1 << infoStyle;
      }
      *m_out << "\x1b[K\x1b[0m\n";

      m_regions.clear();
    }

    *m_out << m_currentFormatedLine << "\x1b[0m\n";

    if (!m_formats.empty())
    {
      lines.clear();
      for (InfoFormat const& info : m_formats)
      {
        Line& line = selectLine(info.offset);
        line.pushName(info.offset, info.style, nameStyle, info.name, infoStyle);

        for (Line* pline = lines.data(); pline <= &line; ++pline)
        {
          pline->pushGraph(info.offset, info.style, graph, infoStyle);
        }
      }

      *m_out << infoStyle;
      for (Line& line : lines)
      {
        *m_out << line.s1 << "\x1b[K\n" << line.s2 << "\x1b[K\n";
      }

      m_formats.clear();
    }

    *m_out << sep;

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
    buf.add('\0');
    m_currentFormatedLine += buf.data();
    if (m_enableTraceName)
    {
      m_formats.push_back(InfoFormat{offset, format.name(), buf.data()});
    }
  }
  else if (m_enableTraceName)
  {
    m_formats.push_back(InfoFormat{offset, format.name(), {}});
  }

  m_currentFormatedLine += m_currentLine.mid(offset, length);

  if (!isDefaultTextStyle)
  {
    m_currentFormatedLine += m_defautStyle;
  }
}

void VtTraceHighlighting::applyFolding(int offset, int /*length*/, FoldingRegion region)
{
  if (m_enableTraceRegion)
  {
    auto id = region.id();

    if (region.type() == FoldingRegion::End)
    {
      auto it = m_regions.rbegin();
      auto eit = m_regions.rend();
      for (int depth = 0; it != eit; ++it)
      {
        if (it->id == id && it->bindIndex < 0)
        {
          if (it->isEnd)
          {
            ++depth;
          }
          else if (--depth < 0)
          {
            break;
          }
        }
      }

      if (it != eit)
      {
        it->bindIndex = int(m_regions.size());
        int bindIndex = int(&*it - m_regions.data());
        m_regions.push_back(InfoRegion{offset, bindIndex, id, true});
      }
      else
      {
        m_regions.push_back(InfoRegion{-1, offset, id, true});
        if (offset != 0)
        {
          m_regions.push_back(InfoRegion{
            offset, int(m_regions.size()), id, true});
        }
      }
    }
    else
    {
      m_regions.push_back(InfoRegion{offset, -1, id, false});
    }
  }
}

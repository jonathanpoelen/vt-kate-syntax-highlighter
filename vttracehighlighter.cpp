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
#include "mini_buf.hpp"
// #include "vtsyntaxhighlighting_logging.h"

#include <KF5/KSyntaxHighlighting/Definition>
#include <KF5/KSyntaxHighlighting/Format>
#include <KF5/KSyntaxHighlighting/State>
#include <KF5/KSyntaxHighlighting/Theme>

# if BUILD_VT_TRACE_CONTEXT
# include <ksyntax-highlighting/src/lib/state_p.h>
# include <ksyntax-highlighting/src/lib/context_p.h>
#endif

//#include <QDebug>
#include <QVector>
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

  template<class T>
  QLatin1String toLatin1(T const& a) noexcept
  {
    return QLatin1String(a.data(), a.size());
  }
}

struct VtTraceHighlighting::InfoFormat
{
  QString name;
  int offset;
  quint16 styleIndex;
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

void VtTraceHighlighting::enableNameTrace(bool withName)
{
  m_enableNameTrace = withName;
}

void VtTraceHighlighting::enableRegionTrace(bool withRegion)
{
  m_enableRegionTrace = withRegion;
}

#if BUILD_VT_TRACE_CONTEXT
void VtTraceHighlighting::enableContextTrace(bool withContext)
{
  m_enableContextTrace = withContext;
}
#endif

void VtTraceHighlighting::highlight()
{
  initStyle();

  QString _colorBuf;
  QStringRef infoStyle;
  QStringRef nameStyle;
  QStringRef idStyle;
  QStringRef graph;
  QStringRef closeGraph;
  QStringRef sep;

  {
    const auto isColor256 = IsColor256(m_isColor256);
    MiniBuf<256> buf;
    int namePos = buf
      .add("\x1b[0;")
      .addBgColor(QColor(34, 34, 34), isColor256)
      .add('m')
      .size()
    ;
    int idPos = buf
      .add("\x1b[7;2m")
      .size()
    ;
    int graphPos = buf
      .add("\x1b[7;3;2m")
      .size()
    ;
    int closeGraphPos = buf
      .add("\x1b[21;23;24;2m|")
      .size()
    ;
    int sepPos = buf
      .add("\x1b[21;23;24;1m|")
      .size()
    ;
    int endPos = buf
      .add("\x1b[")
      .addBgColor(QColor(66, 66, 66), isColor256)
      .add("m────····\x1b[K\x1b[0m\n")
      .size()
    ;

    _colorBuf = QString::fromUtf8(buf.data(), buf.size());
    auto ref = [&](int p1, int p2){
      return _colorBuf.midRef(p1, p2-p1);
    };

    infoStyle = ref(0, namePos);
    nameStyle = ref(namePos, idPos);
    idStyle = ref(idPos, graphPos);
    graph = ref(graphPos, closeGraphPos);
    closeGraph = ref(closeGraphPos, sepPos);
    sep = ref(sepPos, endPos);
  }

  std::vector<Line> lines;

  auto selectLine = [&](int offset) -> Line& {
    auto p = std::find_if(begin(lines), end(lines), [=](Line const& line) {
      return line.endNameLen < offset;
    });
    return (p == end(lines)) ? lines.emplace_back() : *p;
  };

  QString regionName;

  while (!m_in->atEnd())
  {
    m_currentLine = m_in->readLine();
    m_currentFormatedLine.clear();
    m_state = highlightLine(m_currentLine, m_state);

    const bool hasRegion = !m_regions.empty();

    if (hasRegion)
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
            regionName += QLatin1Char(')');
            regionName += tmp;
          }
          else
          {
            regionName.prepend(QLatin1Char(')'));
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
          line.pushGraph(0, QString(), closeGraph, infoStyle);
        }
      }

      for (InfoRegion& info : m_regions)
      {
        if (info.offset < 0)
        {
          continue;
        }

        Line* pline;
        QStringRef graphStyle;

        regionName.setNum(info.id);
        if (info.isEnd)
        {
          pline = &lines[m_regions[info.bindIndex].bindIndex];
          graphStyle = closeGraph;
        }
        else
        {
          regionName += QLatin1Char('(');
          if (info.bindIndex >= 0) {
            int n = m_regions[info.bindIndex].offset - info.offset - regionName.size();
            if (n > 0)
            {
              expandLine(regionName, n, continuationLine);
            }
            regionName += QLatin1Char(')');
          }
          pline = &selectLine(info.offset);
          pline->pushName(info.offset, QString(), idStyle, regionName, infoStyle);
          graphStyle = graph;
          info.bindIndex = pline - lines.data();
        }

        for (Line* p = lines.data(); p <= pline; ++p)
        {
          p->pushGraph(info.offset, QString(), graphStyle, infoStyle);
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

    if (!m_formats.empty())
    {
      lines.clear();
      for (InfoFormat const& info : m_formats)
      {
        Line& line = selectLine(info.offset);
        auto style = toLatin1(m_styles[info.styleIndex]);
        line.pushName(info.offset, style, nameStyle, info.name, infoStyle);

        for (Line* pline = lines.data(); pline <= &line; ++pline)
        {
          pline->pushGraph(info.offset, style, graph, infoStyle);
        }
      }

      auto it = lines.begin();

      if (!hasRegion)
      {
        *m_out << infoStyle;
        for (auto middle = lines.begin() + lines.size()/2; it != middle;)
        {
          --middle;
          *m_out << middle->s2 << "\x1b[K\n" << middle->s1 << "\x1b[K\n";
        }

        it = lines.begin() + lines.size()/2;
      }

      *m_out << m_defautStyle << m_currentFormatedLine << "\x1b[0m\n";

      *m_out << infoStyle;
      for (auto end = lines.end(); it != end; ++it)
      {
        *m_out << it->s1 << "\x1b[K\n" << it->s2 << "\x1b[K\n";
      }

      m_formats.clear();
    }
    else
    {
      *m_out << m_defautStyle << m_currentFormatedLine << "\x1b[0m\n";
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
  const auto id = format.id();
  const auto& style = m_styles[id];
  const bool isDefaultTextStyle = style.isNull();

#if BUILD_VT_TRACE_CONTEXT
  if (m_enableNameTrace || m_enableContextTrace)
  {
    auto getName = [&]{
      if (!m_enableContextTrace) {
        return format.name();
      }

      QString name = QStringLiteral("[");
      if (m_enableContextTrace)
      {
        auto stateData = StateData::get(m_state);
        if (!stateData->isEmpty()) {
          name += stateData->topContext()->name();
        }
      }
      name += QLatin1Char(']');

      if (m_enableNameTrace)
      {
        name += format.name();
      }

      return name;
    };

    m_formats.push_back(InfoFormat{getName(), offset, id});
  }
#else
  if (m_enableNameTrace)
  {
    m_formats.push_back(InfoFormat{format.name(), offset, id});
  }
#endif

  if (!isDefaultTextStyle)
  {
    *m_out << style;
  }

  if (!isDefaultTextStyle)
  {
    m_currentFormatedLine += toLatin1(style);
  }

  m_currentFormatedLine += m_currentLine.mid(offset, length);

  if (!isDefaultTextStyle)
  {
    m_currentFormatedLine += toLatin1(m_defautStyle);
  }
}

void VtTraceHighlighting::applyFolding(int offset, int /*length*/, FoldingRegion region)
{
  if (m_enableRegionTrace)
  {
    auto id = region.id();

    if (region.type() == FoldingRegion::End)
    {
      // search open region
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
        // region without open bindIndex = offset
        m_regions.push_back(InfoRegion{-1, offset, id, true});
        if (offset != 0)
        {
          m_regions.push_back(InfoRegion{
            offset, int(m_regions.size())-1, id, true});
        }
      }
    }
    else
    {
      m_regions.push_back(InfoRegion{offset, -1, id, false});
    }
  }
}

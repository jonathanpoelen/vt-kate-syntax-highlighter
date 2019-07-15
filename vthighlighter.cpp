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

#include "vthighlighter.h"
#include "mini_buf.hpp"
// #include "vtsyntaxhighlighting_logging.h"

#include <ksyntax-highlighting/src/lib/definition.h>
#include <ksyntax-highlighting/src/lib/format.h>
#include <ksyntax-highlighting/src/lib/state.h>
#include <ksyntax-highlighting/src/lib/theme.h>

//#include <QDebug>
#include <QTextStream>
//#include <QLoggingCategory>
#include <QVector>


using namespace VtSyntaxHighlighting;
using namespace KSyntaxHighlighting;

VtHighlighter::VtHighlighter() = default;
VtHighlighter::~VtHighlighter() = default;

void VtHighlighter::setInputStream(QTextStream & in)
{
  m_in = &in;
}

void VtHighlighter::setOutputStream(QTextStream & out)
{
  m_out = &out;
}

void VtHighlighter::useDefaultStyle(bool used)
{
  m_useDefaultStyle = used;
}

void VtHighlighter::enableBuffer(bool isBuffered)
{
  m_isBuffered = isBuffered;
}

void VtHighlighter::initStyle()
{
  if (!m_out)
  {
    //qCWarning(Log) << "No output stream defined!";
    return;
  }

  if (!m_in)
  {
    //qCWarning(Log) << "No input stream defined!";
    return;
  }

  auto theme = this->theme();
  auto definition = this->definition();
  auto formats = definition.formats();

  auto definitions = definition.includedDefinitions();
  definitions.append(definition);

  m_styles.resize(definitions.size() * 10);

  for (auto&& d : definitions)
  {
    for (auto&& format : d.formats())
    {
      bool isDefaultTextStyle = format.isDefaultTextStyle(theme);

      if (!isDefaultTextStyle)
      {
        MiniBuf<64> buf;

        buf.add("\x1b[");

        if (format.hasTextColor(theme))
        {
          buf.add("38;2;");
          buf.addColor(format.textColor(theme));
        }

        if (format.hasBackgroundColor(theme))
        {
          buf.add(";48;2;");
          buf.addColor(format.backgroundColor(theme));
        }

        if (format.isBold(theme)) buf.add(";1");
        if (format.isItalic(theme)) buf.add(";3");
        if (format.isUnderline(theme)) buf.add(";4");
        if (format.isStrikeThrough(theme)) buf.add(";9");

        buf.add('m');

        auto id = format.id();
        if (id >= m_styles.size())
        {
          m_styles.resize(std::max(size_t(id), m_styles.size() * 2u));
        }
        m_styles[id].append(buf.data(), buf.size());
      }
    }
  }

  MiniBuf<64> defaultStyleBuffer;

  if (m_useDefaultStyle)
  {
    QColor fg = theme.textColor(Theme::Normal);
    QColor bg = theme.backgroundColor(Theme::Normal);
    defaultStyleBuffer
      .add("\x1b[0;38;2;")
      .addColor(fg)
      .add(";48;2;")
      .addColor(bg)
      .add('m')
    ;
  }
  else
  {
    defaultStyleBuffer
      .add("\x1b[0m")
    ;
  }

  m_defautStyle = defaultStyleBuffer.to<QByteArray>();
}

void VtHighlighter::highlight()
{
  initStyle();

  State state;
  while (!m_in->atEnd())
  {
    m_currentLine = m_in->readLine();
    state = highlightLine(m_currentLine, state);
    *m_out << '\n';
    if (!m_isBuffered)
    {
      m_out->flush();
    }
  }
}

void VtHighlighter::applyFormat(int offset, int length, const Format& format)
{
  const auto& style = m_styles[format.id()];
  const bool isDefaultTextStyle = style.isNull();

  if (!isDefaultTextStyle)
  {
    *m_out << style;
  }

  *m_out << m_currentLine.midRef(offset, length);

  if (!isDefaultTextStyle)
  {
    *m_out << m_defautStyle;
  }
}

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
#include "theme_buffer.h"
// #include "vtsyntaxhighlighting_logging.h"

#include <KF5/KSyntaxHighlighting/definition.h>
#include <KF5/KSyntaxHighlighting/format.h>
#include <KF5/KSyntaxHighlighting/state.h>
#include <KF5/KSyntaxHighlighting/theme.h>

//#include <QDebug>
#include <QTextStream>
//#include <QLoggingCategory>

#include <string_view>


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

  m_current_theme = theme();

  MiniBuf<64> defaultStyleBuffer;

  if (m_useDefaultStyle)
  {
    QColor fg = m_current_theme.textColor(Theme::Normal);
    QColor bg = m_current_theme.backgroundColor(Theme::Normal);
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

  m_defautStyle = defaultStyleBuffer.add('\0').data();
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
  bool isDefaultTextStyle = format.isDefaultTextStyle(m_current_theme);

  if (!isDefaultTextStyle)
  {
    auto buf = create_vt_theme_buffer(format, m_current_theme);
    buf.add('\0');
    *m_out << buf.data();
  }

  *m_out << m_currentLine.midRef(offset, length);

  if (!isDefaultTextStyle)
  {
    *m_out << m_defautStyle;
  }
}

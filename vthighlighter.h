/*
    Copyright (C) 2017 Jonathan Poelen <jonathan.poelen@gmail.com>

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

#pragma once

//#include <KF5/KSyntaxHighlighting/ksyntaxhighlighting_export.h>
#include <KF5/KSyntaxHighlighting/AbstractHighlighter>
#include <KF5/KSyntaxHighlighting/Theme>

#include <QByteArray>
#include <QString>

#include <vector>


class QTextStream;
class QIODevice;

namespace VtSyntaxHighlighting {

class /*KSYNTAXHIGHLIGHTING_EXPORT*/ VtHighlighter : public KSyntaxHighlighting::AbstractHighlighter
{
public:
  VtHighlighter();
  ~VtHighlighter();

  void setInputStream(QTextStream & in);
  void setOutputStream(QTextStream & out);
  void useDefaultStyle(bool used = true);
  void enableBuffer(bool isUnbuffered = true);
  void enableColor256(bool isColor256 = true);

  void highlight();

protected:
  void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

  void initStyle();

  QTextStream * m_out = nullptr;
  QTextStream * m_in = nullptr;
  std::vector<QByteArray> m_styles;
  QString m_currentLine;
  QByteArray m_defautStyle;
  bool m_isBuffered = true;
  bool m_isColor256 = false;

private:
  bool m_useDefaultStyle = false;
};

}

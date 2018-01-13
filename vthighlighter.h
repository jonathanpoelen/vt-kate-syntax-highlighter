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

#ifndef KSYNTAXHIGHLIGHTING_VT102HIGHLIGHTER_H
#define KSYNTAXHIGHLIGHTING_VT102HIGHLIGHTER_H

//#include <KF5/KSyntaxHighlighting/ksyntaxhighlighting_export.h>
#include <KF5/KSyntaxHighlighting/abstracthighlighter.h>

#include <QString>

class QTextStream;

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
    void enableTraceName(bool withTraceName = true);

    void highlight();

protected:
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

private:
  QTextStream * m_out = nullptr;
  QTextStream * m_in = nullptr;
  QString m_currentLine;
  QString m_defautStyle;
  bool m_useDefaultStyle = false;
  bool m_isBuffered = true;
  bool m_enableTraceName = false;
};

}

#endif // KSYNTAXHIGHLIGHTING_VT102HIGHLIGHTER_H

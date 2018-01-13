/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

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
// #include "vtsyntaxhighlighting_logging.h"

#include <KF5/KSyntaxHighlighting/definition.h>
#include <KF5/KSyntaxHighlighting/format.h>
#include <KF5/KSyntaxHighlighting/state.h>
#include <KF5/KSyntaxHighlighting/theme.h>

//#include <QDebug>
#include <QTextStream>
//#include <QLoggingCategory>

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

void VtHighlighter::enableTraceName(bool withTraceName)
{
    m_enableTraceName = withTraceName;
}

void VtHighlighter::highlight()
{
    if (!m_out) {
        //qCWarning(Log) << "No output stream defined!";
        return;
    }

    if (!m_in) {
        //qCWarning(Log) << "No input stream defined!";
        return;
    }

    if (m_useDefaultStyle) {
        QColor fg = theme().textColor(Theme::Normal);
        QColor bg = theme().backgroundColor(Theme::Normal);
        m_defautStyle = QStringLiteral("\x1b[0;38;2;%1;%2;%3;48;2;%4;%5;%6m")
           .arg(fg.red()).arg(fg.green()).arg(fg.blue())
           .arg(bg.red()).arg(bg.green()).arg(bg.blue());
    }
    else {
        m_defautStyle = QStringLiteral("\x1b[0m");
    }

    State state;
    while (!m_in->atEnd()) {
        m_currentLine = m_in->readLine();
        state = highlightLine(m_currentLine, state);
        *m_out << '\n';
        if (!m_isBuffered) {
            m_out->flush();
        }
    }
}

void VtHighlighter::applyFormat(int offset, int length, const Format& format)
{
    auto&& current_theme = theme();
    bool isDefaultTextStyle = format.isDefaultTextStyle(current_theme);

    if (!isDefaultTextStyle) {
        *m_out << "\x1b[";
        if (format.hasTextColor(current_theme)) {
            QColor color = format.textColor(current_theme);
            *m_out << ";38;2;" << color.red() << ';' << color.green() << ';' << color.blue();
        }
        if (format.hasBackgroundColor(current_theme)) {
            QColor color = format.backgroundColor(current_theme);
            *m_out << ";48;2;" << color.red() << ';' << color.green() << ';' << color.blue();
        }
        if (format.isBold(current_theme))
            *m_out << ";1";
        if (format.isItalic(current_theme))
            *m_out << ";3";
        if (format.isUnderline(current_theme))
            *m_out << ";4";
        if (format.isStrikeThrough(current_theme))
            *m_out << ";9";

        if (m_enableTraceName) {
            *m_out << ";7m"; // inverse
            *m_out << format.name();
            *m_out << "\x1b[27"; // reset inverse
        }
        *m_out << 'm';
    }

    *m_out << m_currentLine.mid(offset, length);

    if (!isDefaultTextStyle)
        *m_out << m_defautStyle;
}

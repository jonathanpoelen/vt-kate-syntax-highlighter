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
// #include "vtsyntaxhighlighting_logging.h"

#include <KF5/KSyntaxHighlighting/definition.h>
#include <KF5/KSyntaxHighlighting/format.h>
#include <KF5/KSyntaxHighlighting/state.h>
#include <KF5/KSyntaxHighlighting/theme.h>

//#include <QDebug>
#include <QTextStream>
//#include <QLoggingCategory>

#include <charconv>
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

namespace
{
    constexpr char const* tb2digits[]{
        "00", "01", "02", "03", "04", "05", "06", "07", "08", "09",
        "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
        "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
        "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
        "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
        "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
        "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
        "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
        "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
        "90", "91", "92", "93", "94", "95", "96", "97", "98", "99",
    };
}

constexpr inline std::size_t vt_rgb_char_size = 3*3+2;

struct MiniBufView
{
#ifndef NDEBUG
    MiniBufView(char* d, char const* e) noexcept
    : m_data(d)
    , m_size(e-d)
    {}

    template<std::size_t N>
    MiniBufView(char (&d)[N]) noexcept
    : m_data(d)
    , m_size(N)
    {}
#else
    MiniBufView(char* d, [[maybe_unused]] char const* e = nullptr) noexcept
    : m_data(d)
    {}
#endif

    MiniBufView& addColor(QColor const& color) noexcept
    {
        assert(m_size - (m_pos - m_data) > std::ptrdiff_t(vt_rgb_char_size));

        auto wc = [&](int x){
            assert(x <= 255 && x >= 0);
            if (x > 99) {
                if (x >= 200) {
                    *m_pos++ = '2';
                    x -= 200;
                }
                else {
                    *m_pos++ = '1';
                    x -= 100;
                }
            }
            else if (x < 10) {
                *m_pos++ = '0' + x;
                return ;
            }

            auto* p = tb2digits[x];
            *m_pos++ = p[0];
            *m_pos++ = p[1];
        };

        wc(color.red());
        *m_pos++ = ';';
        wc(color.green());
        *m_pos++ = ';';
        wc(color.blue());

        return *this;
    }

    MiniBufView& copy(char c) noexcept
    {
        *m_pos++ = c;
        return *this;
    }

    MiniBufView& copy(char const* s) noexcept
    {
        auto len = strlen(s);
        assert(std::size_t(m_pos - m_data) >= len);
        memcpy(m_pos, s, len);
        m_pos += len;
        return *this;
    }

    char const* data() const noexcept
    {
        return m_data;
    }

    std::ptrdiff_t size() const noexcept
    {
        return m_pos - m_data;
    }

    char* charPos() const noexcept
    {
        return m_pos;
    }

private:
    char* m_data;
    char* m_pos = m_data;
#ifndef NDEBUG
    std::size_t m_size;
#endif
};

template<std::size_t N>
struct MiniBuf
{
    MiniBuf& addColor(QColor const& color) noexcept
    {
        m_pos = bufView().addColor(color).charPos();
        return *this;
    }

    MiniBuf& copy(char c) noexcept
    {
        *m_pos++ = c;
        return *this;
    }

    MiniBuf& copy(char const* s) noexcept
    {
        m_pos = bufView().copy(s).charPos();
        return *this;
    }

    char const* data() const noexcept
    {
        return m_data;
    }

    std::ptrdiff_t size() const noexcept
    {
        return m_pos - m_data;
    }

private:
    MiniBufView bufView() const noexcept
    {
        return MiniBufView(m_pos, m_data + N);
    }

    char m_data[N];
    char* m_pos = m_data;
};

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

    m_current_theme = theme();
    m_out_device = m_out->device();

    if (m_useDefaultStyle) {
        QColor fg = m_current_theme.textColor(Theme::Normal);
        QColor bg = m_current_theme.backgroundColor(Theme::Normal);
        m_defautStyleLen = MiniBufView(m_defautStyle)
            .copy("\x1b[0;38;2;")
            .addColor(fg)
            .copy(";48;2;")
            .addColor(bg)
            .copy('m')
            .size()
        ;
    }
    else {
        m_defautStyleLen = MiniBufView(m_defautStyle)
            .copy("\x1b[0m")
            .size()
        ;
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

inline MiniBuf<64> create_vt_theme_buffer(const Format& format, const Theme& theme)
{
    MiniBuf<64> buf;

    buf.copy("\x1b[");

    if (format.hasTextColor(theme)) {
        buf.copy(";38;2;");
        buf.addColor(format.textColor(theme));
    }

    if (format.hasBackgroundColor(theme)) {
        buf.copy(";48;2;");
        buf.addColor(format.backgroundColor(theme));
    }

    if (format.isBold(theme))
        buf.copy(";1");
    if (format.isItalic(theme))
        buf.copy(";3");
    if (format.isUnderline(theme))
        buf.copy(";4");
    if (format.isStrikeThrough(theme))
        buf.copy(";9");

    buf.copy('m');

    return buf;
}

void VtHighlighter::applyFormat(int offset, int length, const Format& format)
{
    bool isDefaultTextStyle = format.isDefaultTextStyle(m_current_theme);

    if (!isDefaultTextStyle) {
        auto buf = create_vt_theme_buffer(format, m_current_theme);
        m_out_device->write(buf.data(), buf.size());
    }

    *m_out << m_currentLine.midRef(offset, length);

    if (!isDefaultTextStyle)
        m_out_device->write(m_defautStyle, m_defautStyleLen);
}

void VtHighlighter::applyFolding(int offset, int length, FoldingRegion region)
{
    (void)offset;
    (void)length;
    (void)region;
}

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

#include <KF5/KSyntaxHighlighting/format.h>
#include <KF5/KSyntaxHighlighting/theme.h>

#include <string_view>
#include <cassert>
#include <cstring>


struct MiniBufView
{
#ifndef NDEBUG
  MiniBufView(char* d, int n) noexcept
  : m_data(d)
  , m_size(n)
  {}

  template<std::size_t N>
  MiniBufView(char (&d)[N]) noexcept
  : m_data(d)
  , m_size(N)
  {}
#else
  MiniBufView(char* d, int n = -1) noexcept
  : m_data(d)
  {}
#endif

  MiniBufView& addColor(QColor const& color) noexcept
  {
    assert(m_size - m_pos > 3*3+2);

    auto wc = [&](int x){
      assert(x <= 255 && x >= 0);
      if (x > 99)
      {
        if (x >= 200)
        {
            add('2');
            x -= 200;
        }
        else
        {
            add('1');
            x -= 100;
        }
      }
      else if (x < 10)
      {
        add('0' + x);
        return ;
      }

      constexpr char const* tb2digits =
        "xx" "xx" "xx" "xx" "xx" "xx" "xx" "xx" "xx" "xx"
        "10" "11" "12" "13" "14" "15" "16" "17" "18" "19"
        "20" "21" "22" "23" "24" "25" "26" "27" "28" "29"
        "30" "31" "32" "33" "34" "35" "36" "37" "38" "39"
        "40" "41" "42" "43" "44" "45" "46" "47" "48" "49"
        "50" "51" "52" "53" "54" "55" "56" "57" "58" "59"
        "60" "61" "62" "63" "64" "65" "66" "67" "68" "69"
        "70" "71" "72" "73" "74" "75" "76" "77" "78" "79"
        "80" "81" "82" "83" "84" "85" "86" "87" "88" "89"
        "90" "91" "92" "93" "94" "95" "96" "97" "98" "99"
      ;
      auto* p = tb2digits + x * 2;
      add(p[0]);
      add(p[1]);
    };

    wc(color.red());
    add(';');
    wc(color.green());
    add(';');
    wc(color.blue());

    return *this;
  }

  MiniBufView& add(char c) noexcept
  {
    m_data[m_pos++] = c;
    return *this;
  }

  MiniBufView& add(std::string_view sv) noexcept
  {
    assert(m_size - m_pos >= int(sv.size()));
    memcpy(m_data + m_pos, sv.data(), sv.size());
    m_pos += int(sv.size());
    return *this;
  }

  char const* data() const noexcept
  {
    return m_data;
  }

  int size() const noexcept
  {
    return m_pos;
  }

private:
  char* m_data;
  int m_pos = 0;
#ifndef NDEBUG
  int m_size;
#endif
};

template<std::size_t N>
struct MiniBuf
{
  MiniBuf& addColor(QColor const& color) noexcept
  {
    m_pos += bufView().addColor(color).size();
    return *this;
  }

  MiniBuf& add(char c) noexcept
  {
    m_data[m_pos++] = c;
    return *this;
  }

  MiniBuf& add(std::string_view sv) noexcept
  {
    m_pos += bufView().add(sv).size();
    return *this;
  }

  char const* data() const noexcept
  {
    return m_data;
  }

  int size() const noexcept
  {
    return m_pos;
  }

private:
  MiniBufView bufView() noexcept
  {
    return MiniBufView(m_data + m_pos, N - m_pos);
  }

  int m_pos = 0;
  char m_data[N];
};

using ThemeFormatBuf = MiniBuf<56>;

inline ThemeFormatBuf create_vt_theme_buffer(const KSyntaxHighlighting::Format& format, const KSyntaxHighlighting::Theme& theme)
{
  ThemeFormatBuf buf;

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

  return buf;
}

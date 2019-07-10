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

#include "vthighlighter.h"
#include <KF5/KSyntaxHighlighting/foldingregion.h>

#include <string>
#include <vector>


class QTextStream;

namespace VtSyntaxHighlighting {

class /*KSYNTAXHIGHLIGHTING_EXPORT*/ VtTraceHighlighting : public VtHighlighter
{
public:
  VtTraceHighlighting();
  ~VtTraceHighlighting();

  void enableTraceName(bool withTraceName = true);
  void enableTraceRegion(bool withTraceRegion = true);

  void highlight();

protected:
  void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;
  void applyFolding(int offset, int length, KSyntaxHighlighting::FoldingRegion region) override;

  struct InfoFormat;
  struct InfoRegion;

  QString m_currentFormatedLine;
  std::vector<InfoFormat> m_formats;
  std::vector<InfoRegion> m_regions;
  bool m_enableTraceName = false;
  bool m_enableTraceRegion = false;
};

}

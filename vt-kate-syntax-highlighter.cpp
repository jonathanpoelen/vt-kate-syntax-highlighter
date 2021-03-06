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

#ifndef BUILD_VT_TRACE
# define BUILD_VT_TRACE 0
#endif

#if BUILD_VT_TRACE
# include "vttracehighlighter.h"
#else
# include "vthighlighter.h"
#endif

#include <KF5/KSyntaxHighlighting/definition.h>
#include <KF5/KSyntaxHighlighting/repository.h>
#include <KF5/KSyntaxHighlighting/theme.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QVector>

#include <iostream>

using namespace KSyntaxHighlighting;
using namespace VtSyntaxHighlighting;

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName(QStringLiteral("vt-kate-syntax-highlighter"));
  QCoreApplication::setOrganizationName(QStringLiteral("Jonathan Poelen"));
  QCoreApplication::setApplicationVersion(QStringLiteral("1.0.1"));

  QCommandLineParser parser;
  parser.setApplicationDescription(app.translate("SyntaxHighlightingCLI", "Command line syntax highlighter using Kate syntax definitions."));
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument(app.translate("SyntaxHighlightingCLI", "[source]"),
                               app.translate("SyntaxHighlightingCLI", "The source file to highlight. Otherwise, use stdin and require -s."));

  QCommandLineOption listDefs(QStringList() << QStringLiteral("l") << QStringLiteral("list"),
                              app.translate("SyntaxHighlightingCLI", "List all available syntax definitions."));
  parser.addOption(listDefs);
  QCommandLineOption listThemes(QStringList() << QStringLiteral("list-themes"),
                                app.translate("SyntaxHighlightingCLI", "List all available themes."));
  parser.addOption(listThemes);

  QCommandLineOption outputName(QStringList() << QStringLiteral("o") << QStringLiteral("output"),
                                app.translate("SyntaxHighlightingCLI", "File to write ANSI output to (default: stdout)."),
                                app.translate("SyntaxHighlightingCLI", "output"));
  parser.addOption(outputName);

  QCommandLineOption syntaxName(QStringList() << QStringLiteral("s") << QStringLiteral("syntax"),
                                app.translate("SyntaxHighlightingCLI", "Highlight using this syntax definition (default: auto-detect based on input file)."),
                                app.translate("SyntaxHighlightingCLI", "syntax"));
  parser.addOption(syntaxName);

  QCommandLineOption themeName(QStringList() << QStringLiteral("t") << QStringLiteral("theme"),
                               app.translate("SyntaxHighlightingCLI", "Color theme to use for highlighting."),
                               app.translate("SyntaxHighlightingCLI", "theme"), QStringLiteral("Default"));
  parser.addOption(themeName);

  QCommandLineOption useDefaultStyle(QStringList() << QStringLiteral("d") << QStringLiteral("default-style"),
                                     app.translate("SyntaxHighlightingCLI", "Use default color style."));
  parser.addOption(useDefaultStyle);

  QCommandLineOption unbuffered(QStringList() << QStringLiteral("u") << QStringLiteral("unbuffered"),
                                app.translate("SyntaxHighlightingCLI", "Flush on each line."));
  parser.addOption(unbuffered);

  QCommandLineOption useColor256(QStringList() << QStringLiteral("c") << QStringLiteral("color256"),
                                app.translate("SyntaxHighlightingCLI", "use 256 colors rather than TrueColor."));
  parser.addOption(useColor256);

#if BUILD_VT_TRACE
  QCommandLineOption enableNameTrace(QStringList() << QStringLiteral("n") << QStringLiteral("name"),
                                     app.translate("SyntaxHighlightingCLI", "Add the format name on each color."));
  parser.addOption(enableNameTrace);

  QCommandLineOption enableRegionTrace(QStringList() << QStringLiteral("r") << QStringLiteral("region"),
                                       app.translate("SyntaxHighlightingCLI", "Add region id."));
  parser.addOption(enableRegionTrace);

#if BUILD_VT_TRACE_CONTEXT
  QCommandLineOption enableContextTrace(QStringList() << QStringLiteral("x") << QStringLiteral("context"),
                                       app.translate("SyntaxHighlightingCLI", "Add context of format."));
  parser.addOption(enableContextTrace);
#endif
#endif

  parser.process(app);

  Repository repo;
  if (parser.isSet(listDefs))
  {
    for (auto const& def : repo.definitions())
    {
      std::cout << qPrintable(def.name()) << "  --";
      for (auto const& extension : def.extensions())
      {
        std::cout << ' ' << qPrintable(extension);
      }
      std::cout << std::endl;
    }
    return 0;
  }

  if (parser.isSet(listThemes))
  {
    for (const auto &theme : repo.themes())
    {
      std::cout << qPrintable(theme.name()) << std::endl;
    }
    return 0;
  }

  QStringList positionalArguments = parser.positionalArguments();
  if (positionalArguments.size() > 1)
  {
    parser.showHelp(1);
  }

  Definition def;
  if (parser.isSet(syntaxName))
  {
    auto syntax = parser.value(syntaxName);
    def = repo.definitionForName(syntax);
    if (!def.isValid())
    {
      def = repo.definitionForFileName(QLatin1String("f.")+syntax);
      if (!def.isValid())
      {
        def = repo.definitionForFileName(syntax);
      }
    }
  }
  else if (positionalArguments.size())
  {
    def = repo.definitionForFileName(positionalArguments.at(0));
  }
  else
  {
    std::cerr << "Missing syntax option.\n" << std::endl;
    parser.showHelp(1);
  }

  if (!def.isValid())
  {
    std::cerr << "Unknown syntax." << std::endl;
    return 1;
  }

  QFile outFile;
  QFile inFile;

  if (parser.isSet(outputName))
  {
    auto outputFileName = parser.value(outputName);
    outFile.setFileName(outputFileName);
    if (!outFile.open(QFile::WriteOnly | QFile::Truncate))
    {
      std::cerr << "Failed to open output file" << outputFileName.toStdString() << ":" << outFile.errorString().toStdString();
      return 2;
    }
  }
  else
  {
    outFile.open(stdout, QFile::WriteOnly);
  }

  if (positionalArguments.size())
  {
    inFile.setFileName(positionalArguments.at(0));
    if (!inFile.open(QFile::ReadOnly))
    {
      std::cerr << "Failed to open input file" << positionalArguments.at(0).toStdString() << ":" << inFile.errorString().toStdString();
      return 3;
    }
  }
  else
  {
    inFile.open(stdin, QFile::ReadOnly);
  }

  QTextStream out;
  QTextStream in;

  out.setDevice(&outFile);
  in.setDevice(&inFile);

  out.setCodec("UTF-8");
  in.setCodec("UTF-8");

#if BUILD_VT_TRACE
  VtTraceHighlighting highlighter;
  highlighter.enableNameTrace(parser.isSet(enableNameTrace));
  highlighter.enableRegionTrace(parser.isSet(enableRegionTrace));
#if BUILD_VT_TRACE_CONTEXT
  highlighter.enableContextTrace(parser.isSet(enableContextTrace));
#endif
#else
  VtHighlighter highlighter;
#endif
  highlighter.setInputStream(in);
  highlighter.setOutputStream(out);
  highlighter.setDefinition(def);
  highlighter.enableColor256(parser.isSet(useColor256));
  highlighter.setTheme(repo.theme(parser.value(themeName)));
  highlighter.useDefaultStyle(parser.isSet(useDefaultStyle));
  highlighter.enableBuffer(!parser.isSet(unbuffered));

  highlighter.highlight();

  return 0;
}

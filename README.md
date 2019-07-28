# Ansi Syntax Highlighting

<!-- toc -->
<!-- /toc -->

This is a simple highlighted text rendering to Vt100 based of the [Kate syntax highlighting engine](https://phabricator.kde.org/source/syntax-highlighting/).

## Dependencies

- KF5SyntaxHighlighting

The package name vary between distributions:

- Arch Linux: `syntax-highlighting` in extra
- Debian / Ubuntu: `libkf5syntaxhighlighting-dev`
- Gentoo: `libkf5syntaxhighlighting-devel`

## Build

```bash
mkdir bin &&
cd bin &&
cmake -DCMAKE_BUILD_TYPE=Release .. &&
make -j2
```

### Enable VtTrace

VtTrace is a mode used to debug KF5SyntaxHighlighting syntax files more easily, it is activated with `-DBUILD_VT_TRACE=ON`. To display the current context, add `-DBUILD_VT_TRACE_CONTEXT=ON` and make a symbolic link from `ksyntax-highlighting` to the official KF5SyntaxHighlighting sources or clone the repository directly.

```bash
git clone git://anongit.kde.org/syntax-highlighting ksyntax-highlighting
```

## How to use my personal theme?

A custom theme can be used by setting the environment variable `XDG_DATA_DIRS` to a folder that contains `org.kde.syntax-highlighting/themes`. The folder in the repository contains the theme "Breeze Dark" with custom colors for some languages. Syntax files will need to be added manually to a `org.kde.syntax-highlighting/syntax` folder via eg a symlink to your system.

```bash
ln -s /usr/share/org.kde.syntax-highlighting/syntax org.kde.syntax-highlighting
```

Run `vt-kate-syntax-highlighter` with `XDG_DATA_DIRS=/path/of/repo vt-kate-syntax-highlighter -t'My Breeze Dark'`.

`My Breeze Dark` refers to the `name` value in the theme file.

You can also create aliases in your `~/.bashrc`:

```bash
alias hi='XDG_DATA_DIRS=/path/of/repo vt-kate-syntax-highlighter -tMy\ Breeze\ Dark'
alias ihi='hi -s'
```

## How to import the Kate's color?

The following commands create a theme from the colors configured in Kate.

(See the previous chapter to configure a theme)

```bash
./katesyntaxhighlightingrc2themes.sh ${KATE_SYNTAX_HIGHLIGHTING_RC}
./mergetheme.lua ${HL_THEME} \
  ./pre-themes/Default\ Item\ Styles\ -\ Schema\ ${KATE_SCHEMA_NAME}.json \
  ./pre-themes/Highlighting\ *\ -\ Schema\ {KATE_SCHEMA_NAME}.json \
  > ${KDE_DATA_PATH}/org.kde.syntax-highlighting/themes/my.theme
```

- `${KATE_SYNTAX_HIGHLIGHTING_RC}`: Can be omitted, by default `~/.config/katesyntaxhighlightingrc`.
- `${HL_THEME}`: A theme in [KSyntaxHighlighter project](https://phabricator.kde.org/source/syntax-highlighting/browse/master/data/themes/).
- `${KDE_DATA_PATH}`: The KDE data directory. Candidats listed by `qtpaths --paths GenericDataLocation` command. Usually `~/.local/share`, configurable with the environment variable `$XDG_DATA_DIRS`.

Note: For renamed the theme, open my.theme, search and replace `"My"` with what you want. (Use `json_pp` to format the source `json_pp < infile > outfile`).

Note: `./mergetheme.lua` requires `lua` and `lua-json` package.

### Example

```bash
D=~/game/org.kde.syntax-highlighting/themes/
mkdir -p "$D"
./katesyntaxhighlightingrc2themes.sh
./mergetheme.lua \
  <(wget -O- https://phabricator.kde.org/file/data/rt2qwxjdmc4iq5aq6mid/PHID-FILE-lnzfavt566nzip4qmcsh/breeze-dark.theme) \
  ./pre-themes/Default\ Item\ Styles\ -\ Schema\ Breeze\ Dark.json \
  ./pre-themes/Highlighting\ *\ -\ Schema\ Breeze\ Dark.json \
  >"$D"/my.theme
rm -r ./pre-themes
```

## Similar Project

- libkf5syntaxhighlighting-tools - [kate-syntax-highlighter: HTML rendering](https://phabricator.kde.org/source/syntax-highlighting/browse/master/src/cli/)

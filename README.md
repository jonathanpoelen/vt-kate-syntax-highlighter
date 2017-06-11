# Ansi Syntax Highlighting

This is a simple highlighted text rendering to Vt100 based of the [Kate syntax highlighting engine](https://phabricator.kde.org/source/syntax-highlighting/).

## Dependencies

- libkf5syntaxhighlighting-dev or libkf5syntaxhighlighting-devel
- libkf5syntaxhighlighting5

## Build

```bash
mkdir bin && cd bin
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j2
```

## How to use the Kate's color ?

```bash
./katesyntaxhighlightingrc2themes.sh ${KATE_SYNTAX_HIGHLIGHTING_RC}
./mergetheme.lua ${HL_THEME} \
  ./pre-themes/Default\ Item\ Styles\ -\ Schema\ ${KATE_SCHEMA_NAME}.json \
  ./pre-themes/Highlighting\ *\ -\ Schema\ {KATE_SCHEMA_NAME}.json \
  > ${KDE_DATA_PATH}/org.kde.syntax-highlighting/themes/my.theme
```

- `${KATE_SYNTAX_HIGHLIGHTING_RC}`: Can be omitted, by default `$HOME/.config/katesyntaxhighlightingrc`.
- `${HL_THEME}`: A theme in [KSyntaxHighlighter project](https://phabricator.kde.org/source/syntax-highlighting/browse/master/data/themes/).
- `${KDE_DATA_PATH}`: The KDE data directory. Candidats listed by `qtpaths --paths GenericDataLocation` command.

Note: For renamed the theme, open my.theme, search and replace `"My"` with what you want. (Use `json_pp` to format the source `json_pp < infile > outfile`).

Note: `./mergetheme.lua` requires `lua` and `lua-json` package.

### Example

```bash
D=~/game/org.kde.syntax-highlighting/themes/
mkdir -p $D
./katesyntaxhighlightingrc2themes.sh
./mergetheme.lua \
  <(wget -O- https://phabricator.kde.org/file/data/rt2qwxjdmc4iq5aq6mid/PHID-FILE-lnzfavt566nzip4qmcsh/breeze-dark.theme) \
  ./pre-themes/Default\ Item\ Styles\ -\ Schema\ Breeze\ Dark.json \
  ./pre-themes/Highlighting\ *\ -\ Schema\ Breeze\ Dark.json \
  >$D/my.theme
rm -rf ./pre-themes
```

Run with `XDG_DATA_DIRS=~/game ./vt-kate-syntax-highlighter -tMy`.


## Similar Project

- libkf5syntaxhighlighting-tools - [kate-syntax-highlighter: HTML rendering](https://phabricator.kde.org/source/syntax-highlighting/browse/master/src/cli/)

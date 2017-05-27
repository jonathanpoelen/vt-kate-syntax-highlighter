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

## Similar Project

- libkf5syntaxhighlighting-tools - [kate-syntax-highlighter: HTML rendering](https://phabricator.kde.org/source/syntax-highlighting/browse/master/src/cli/)

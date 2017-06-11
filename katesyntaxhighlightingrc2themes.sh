#!/bin/bash

hlrc=${1:-$HOME/.config/katesyntaxhighlightingrc}
typeset -A hl=()

color ()
{
  v=${a[$(($style+$2))]}
  [ ! -z "$v" ] && s+=',"'$1'":"#'${v:2}'"'
}

bool ()
{
  v=${a[$(($style+$2))]}
  if [ ! -z "$v" ] ; then
    s+=',"'$1'":'
    [ "$v" = 1 ] && s+=true || s+=false
  fi
}

style=
name=
while read l ; do
  [ ${#l} -eq 0 ] && continue
  if [ "${l:0:1}" = '[' ] ; then
    # [Default Item Styles - {style}] or [Highlighting {syntax} - {style}]
    [ "${l:1:1}" = 'D' ] && style=0 || style=1
    len=$((${#l}-2))
    name=${l:1:$len}
    continue
  fi

  IFS==, read -a a <<<"$l"
  # ktexteditor/src/syntax/katesyntaxmanager.cpp
  # name,color,_,bold,italic,strikeout,underline,background|-

  s=
  color text-color 1
  bool bold 3
  bool italic 4
  bool underline 5
  bool strike-through 6
  [ "${a[$(($style+7))]}" != - ] && color background-color 7

  [ -z "$s" ] && continue

  if [ $style -eq 0 ] ; then
    hl[$name]+=",\"${a[0]}\":{${s:1}}"$'\n'
  else
    IFS=: read lang ds <<<"${a[0]}"
    hl[$name]+=",\"$ds\":{\"language\":\"$lang\",\"inherit\":${a[1]}$s}"$'\n'
  fi
done < "$hlrc"

mkdir -p pre-themes
for k in "${!hl[@]}" ; do
  echo "{ ${hl[$k]:1}}" > pre-themes/"$k".json
done

#!/usr/bin/env lua

argc=table.getn(arg)
if argc < 2 then
  print(arg[0] .. " file_theme file_theme_overrided...")
  os.exit(1)
end

json=require('json')

function readjson(fileName)
  local f, err = io.open(fileName)
  if not f then
    io.stderr:write(err..'\n')
    os.exit(2)
  end
  return json.decode(f:read('*a'))
end

themes={}
master=readjson(arg[1])
for i=1, argc-1 do
  themes[i-1]=readjson(arg[i+1])
end

-- ktexteditor/src/syntax/katesyntaxmanager.cpp
dsNames={
  'Normal',
  'Keyword',
  'Function',
  'Variable',
  'ControlFlow',
  'Operator',
  'BuiltIn',
  'Extension',
  'Preprocessor',
  'Attribute',
  'Char',
  'SpecialChar',
  'String',
  'VerbatimString',
  'SpecialString',
  'Import',
  'DataType',
  'DecVal',
  'BaseN',
  'Float',
  'Constant',
  'Comment',
  'Documentation',
  'Annotation',
  'CommentVar',
  'RegionMarker',
  'Information',
  'Warning',
  'Alert',
  'Others',
  'Error',
}
normalized={
  ["Normal"]='Normal',
  ["Keyword"]='Keyword',
  ["Function"]='Function',
  ["Variable"]='Variable',
  ["Control Flow"]='ControlFlow',
  ["Operator"]='Operator',
  ["Built-in"]='BuiltIn',
  ["Extension"]='Extension',
  ["Preprocessor"]='Preprocessor',
  ["Attribute"]='Attribute',
  ["Character"]='Char',
  ["Special Character"]='SpecialChar',
  ["String"]='String',
  ["Verbatim String"]='VerbatimString',
  ["Special String"]='SpecialString',
  ["Import"]='Import',
  ["Data Type"]='DataType',
  ["Decimal/Value"]='DecVal',
  ["Base-N Integer"]='BaseN',
  ["Floating Point"]='Float',
  ["Constant"]='Constant',
  ["Comment"]='Comment',
  ["Documentation"]='Documentation',
  ["Annotation"]='Annotation',
  ["Comment Variable"]='CommentVar',
  ["Region Marker"]='RegionMarker',
  ["Information"]='Information',
  ["Warning"]='Warning',
  ["Alert"]='Alert',
  ["Others"]='Others',
  ["Error"]='Error'
}

master['custom-styles']={}
dsStyles=master['text-styles']
custom=master['custom-styles']

for k=0,#themes do
  for style,def in pairs(themes[k]) do
    norm = normalized[style]
    if nil == def.language then
      dsStyles[norm] = def
    else
      inheritNorm = dsNames[def.inherit]
      o = {}
      if nil ~= dsStyles[inheritNorm] then
        o = {unpack(dsStyles[inheritNorm])}
      end
      for k,v in pairs(def) do
        o[k] = v
      end
      o.inherit=nil
      o.language=nil

      if nil == custom[def.language] then
        custom[def.language]={}
      end
      custom[def.language][norm or style]=o
    end
  end
end

master.metadata.name = "My"
print(json.encode(master))

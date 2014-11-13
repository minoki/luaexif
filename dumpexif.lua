local exif = require "exif"
local file = assert(arg[1],"no file given")
local data = exif.loadfile(file)
for _,content in ipairs(data:ifds()) do
  assert(content == data:ifd(content.ifd))
  assert(content.parent == data)
  io.write("IFD ",content.ifd,":\n")
  for _,entry in ipairs(content:entries()) do
    assert(entry == content:entry(entry.tag))
    assert(entry.parent == content)
    assert(entry.value == tostring(entry))
    assert(type(entry.components) == "number")
    assert(type(entry.format) == "string")
    assert(type(entry.rawdata) == "string")
    io.write("\t",entry.tag,"\t",entry.value,"\n")
  end
end
do
  local mnotedata = data.mnotedata
  if mnotedata then
    io.write("MakerNotes:\n")
    for i = 1,#mnotedata do
      local x = mnotedata[i]
      io.write(string.format("\t[#%d %s] %s (%s)\t%s\n",
        x.tagid,
        x.tag or "<unknown>",
        x.title or "<unknown>",
        x.description or "<unknown>",
        x.value))
    end
  end
end

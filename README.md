luaexif
=======

This is a [Lua](http://www.lua.org/) binding for [libexif](http://libexif.sourceforge.net/).
Compatible with Lua 5.1 and Lua 5.2.

Reference
=========
* The `exif` module
    * `exif.new()`
      Allocates a new `data` and returns it.
    * `exif.loadfile(filename)`
      Loads the file specified by `filename`. Returns a `data`.
    * `exif.loadbuffer(buffer)`
      Loads the buffer given by `buffer`. Returns a `data`.
* `data` --- the entire EXIF data found in an image.
    * `data.mnotedata`
      Returns the MakerNote data.
    * `data:fix()`
    * `data:loadbuffer(buffer)`
    * `data:ifd(ifd)`
      `ifd` is one of `"0"`,`"1"`,`"EXIF"`,`"GPS"`,`"Interoperability"`.
      Returns an IFD.
    * `data:ifds()`
      Returns a number-indexed table that contains all IFDs.
* `content` --- all EXIF tags found in a single IFD.
    * `content:fix()`
    * `content.ifd` Returns one of `"0"`,`"1"`,`"EXIF"`,`"GPS"`,`"Interoperability"`.
    * `content:entry(tag)`
      Returns an EXIF tag.
    * `content:entries()`
      Returns a table that contains all EXIF tags.
    * `content.parent`
* `entry` --- one EXIF tag
    * `entry:fix()`
    * `entry.tag`
    * `entry.value`
    * `entry.components`
    * `entry.format`
    * `entry.rawdata`
    * `entry.parent`
    * `entry.data`
    * `entry[n]` `n` is an integer.
    * `tostring(entry)`
* `mnotedata` --- all data found in MakerNote tag.
    * `#mnotedata`
    * `mnotedata[n]` `n` is an integer between `1` and `#mnotedata`.
* `rational`
    * `rational.numerator`
    * `rational.denominator`
    * `rational.value`
    * `tostring(rational)`

Example
=======
```Lua
local exif = require "exif"
local data = exif.loadfile("mypicture.jpg") -- data is 
print(data:ifd"0":entry"DateTime") --> "2014:10:25 12:55:22"
```

Note that requiring "exif" module doesn't set a global variable.
You can assign the module (result of `require`) to a local variable, as in the example above.

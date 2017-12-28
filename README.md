luaexif
=======

This is a [Lua](https://www.lua.org/) binding for [libexif](https://libexif.github.io/).
Compatible with Lua 5.1 or later.

Install
=======

```
$ luarocks make LIBEXIF_DIR=/usr/local
```

Or, manually with Makefile:
```
$ make LUA=/usr/local LIBEXIF=/usr LDFLAGS=-shared
$ cp exif.so /path/to/module/dir/
```

Manual
======
The `exif` module:

* `exif = require "exif"`
  Loads the module and set it to a variable. Note that the call to `require` does not set a global variable.
* `exif.new()`
  Allocates a new `data` and returns it.
* `exif.loadfile(filename)`
  Loads the file specified by `filename`. Returns a `data`.
* `exif.loadbuffer(buffer)`
  Loads the buffer given by `buffer`. Returns a `data`.

Data types:

* `data` --- the entire EXIF data found in an image.
    * `mnotedata = data.mnotedata`
      Returns the MakerNote data.
    * `data:fix()`
    * `data:loadbuffer(buffer)`
    * `content = data:ifd(ifd)`
      `ifd` is one of `"0"`,`"1"`,`"EXIF"`,`"GPS"`,`"Interoperability"`.
      Returns an IFD.
    * `data:ifds()`
      Returns a integer-indexed table that contains all IFDs.
* `content` --- all EXIF tags found in a single IFD.
    * `content:fix()`
    * `content.ifd`
      Returns one of `"0"`,`"1"`,`"EXIF"`,`"GPS"`,`"Interoperability"`.
    * `entry = content:entry(tag)`
      Returns an EXIF tag.
    * `content:entries()`
      Returns a table that contains all EXIF tags.
    * `content.parent`
* `entry` --- one EXIF tag
    * `entry:fix()`
    * `entry.tag` Tag name as a string.
    * `entry.value` Returns a localized textual representation of the value.
    * `entry.components`
    * `entry.format` Returns a textual representation of the data type.
    * `entry.rawdata`
    * `entry.parent` Returns the content object to which `entry` belongs.
    * `entry.data` Same as `entry[1]`
    * `entry[n]` (`n` is an integer between `1` and `entry.components`)
    * `tostring(entry)` Same as `entry.value`.
* `mnotedata` --- all data found in MakerNote tag.
    * `#mnotedata`
    * `mnotedata[n]`
      `n` is an integer between `1` and `#mnotedata`.
* `rational` --- a rational number, possibly returned by `entry.data` or `entry[n]`
    * `rational.numerator` The numerator as an integer.
    * `rational.denominator` The denominator as an integer.
    * `rational.value` The ratio as a Lua number.
    * `tostring(rational)` Same as `rational.numerator .. "/" .. rational.denominator`.

Example
=======
```Lua
local exif = require "exif"
local data = exif.loadfile("mypicture.jpg") -- Load the EXIF data from "mypicture.jpg"
print(data:ifd("0"):entry("DateTime")) --> "2014:10:25 12:55:22"
local ExposureTime = data:ifd("EXIF"):entry("ExposureTime")
print(ExposureTime) --> "1/125 sec."
print(ExposureTime.format) --> "Rational"
print(ExposureTime.data) --> "1/125"
print(ExposureTime.data.value) --> "0.008"
```

Note that requiring "exif" module doesn't set a global variable with the module name.
You can assign the module (result of `require`) to a local variable, as in the example above.

Links
=====

* [Lua](https://www.lua.org/)
* [libexif](https://libexif.github.io/)

## lua-libphonenumber

Lua bindings to the C/C++ <a href="https://github.com/google/libphonenumber/">Google libphonenumber</a> library.
Library for parsing, formatting, and validating international phone numbers.


## Dependencies

Make sure you have installed dependencies according to your OS and lua version:

```bash
sudo apt install build-essential pkgconf liblua5.1-0-dev libicu-dev libphonenumber8 libphonenumber-dev
```

## Installation

```bash
luarocks install lua-libphonenumber
```

## Usage

```Lua
local lpn = require("lua-libphonenumber")

print("parsed", json.encode(lpn.parse("+13104527609")))
```

```
parsed	{"location":"Santa Monica, CA","type":"FIXED_LINE_OR_MOBILE","E164":"+13104527609","INTERNATIONAL":"+1 310-452-7609","NATIONAL":"(310) 452-7609","RFC3966":"tel:+1-310-452-7609","is_possible":true,"is_valid":true,"country_code":1,"region":"US"}
```

More examples in the test.lua file.
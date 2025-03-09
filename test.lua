local json = require("cjson")
local lpn = require("lua-libphonenumber")

print("parsed", json.encode(lpn.parse("+13104527609")))
--parsed	{"location":"Santa Monica, CA","type":"FIXED_LINE_OR_MOBILE","E164":"+13104527609","INTERNATIONAL":"+1 310-452-7609","NATIONAL":"(310) 452-7609","RFC3966":"tel:+1-310-452-7609","is_possible":true,"is_valid":true,"country_code":1,"region":"US"}
print("parsed", json.encode(lpn.parse("+13104527609", "us")))
--parsed	{"location":"Santa Monica, CA","type":"FIXED_LINE_OR_MOBILE","E164":"+13104527609","INTERNATIONAL":"+1 310-452-7609","NATIONAL":"(310) 452-7609","RFC3966":"tel:+1-310-452-7609","is_possible":true,"is_valid":true,"country_code":1,"region":"US"}

print("E164", json.encode(lpn.format("+13104527609", "us", "E164")))
--E164	"+13104527609"

print("INTERNATIONAL", json.encode(lpn.format("+13104527609", "us", "INTERNATIONAL")))
--INTERNATIONAL	"+1 310-452-7609"

print("NATIONAL", json.encode(lpn.format("+13104527609", "us", "NATIONAL")))
--NATIONAL	"(310) 452-7609"

print("RFC3966", json.encode(lpn.format("+13104527609", "us", "RFC3966")))
--RFC3966	"tel:+1-310-452-7609"

print("type", json.encode(lpn.get_number_type("+13104527609")))
--type	"FIXED_LINE_OR_MOBILE"

print("type", json.encode(lpn.get_number_type("+13104527609", "us")))
--type	"FIXED_LINE_OR_MOBILE"

print("is_possible", json.encode(lpn.is_possible_number("+13104527609")))
--is_possible	true

print("is_possible", json.encode(lpn.is_possible_number("+13104527609", "us")))
--is_possible	true

print("is_valid", json.encode(lpn.is_valid_number("+13104527609")))
--is_valid	true

print("is_valid", json.encode(lpn.is_valid_number("+13104527609", "us")))
--is_valid	true

print("region", json.encode(lpn.get_region("+13104527609")))
--region	"US"

print("region", json.encode(lpn.get_region("+13104527609", "us")))
--region	"US"

print("location", json.encode(lpn.get_location("+13104527609", "us")))
--location	"Santa Monica, CA"

print("location", json.encode(lpn.get_location("+13104527609", "us", "en", "US")))
--location	"Santa Monica, CA"

print("as_you_type", json.encode(lpn.as_you_type_formatter("+1310452")))
--as_you_type	"+1 310-452"

print("as_you_type", json.encode(lpn.as_you_type_formatter("+1310452", "us")))
--as_you_type	"+1 310-452"

--path to a directory containing files from https://github.com/google/libphonenumber/tree/master/resources/carrier
print("carrier", json.encode(lpn.get_carrier("+13104527609", "us", "/opt/carrier")))
--carrier	""
print("carrier", json.encode(lpn.get_carrier("+13104527609", "us", "/opt/carrier", "en", "|")))
--carrier	""

print("parsed", json.encode(lpn.parse("+552124984731")))
--parsed	{"location":"Rio de Janeiro - RJ","type":"FIXED_LINE","E164":"+552124984731","INTERNATIONAL":"+55 21 2498-4731","NATIONAL":"(21) 2498-4731","RFC3966":"tel:+55-21-2498-4731","is_possible":true,"is_valid":true,"country_code":55,"region":"BR"}

print("carrier", json.encode(lpn.get_carrier("+552124984731", "us", "/opt/carrier")))
--carrier	""

print("parsed", json.encode(lpn.parse("+34 676882721")))
--parsed	{"location":"Spain","type":"MOBILE","E164":"+34676882721","INTERNATIONAL":"+34 676 88 27 21","NATIONAL":"676 88 27 21","RFC3966":"tel:+34-676-88-27-21","is_possible":true,"is_valid":true,"country_code":34,"region":"ES"}

print("carrier", json.encode(lpn.get_carrier("+34 676882721", "us", "/opt/carrier")))
--carrier	"Vodafone"

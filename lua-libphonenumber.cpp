extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <sys/stat.h>
#include "phonenumbers/phonenumberutil.h"
#include "phonenumbers/phonenumber.pb.h"
#include "phonenumbers/geocoding/phonenumber_offline_geocoder.h"
#include "phonenumbers/asyoutypeformatter.h"

using namespace i18n::phonenumbers;

PhoneNumberUtil* phone_util = PhoneNumberUtil::GetInstance();
PhoneNumberOfflineGeocoder geocoder;

bool FileExists(const std::string& file_path) {
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}

std::map<std::string, std::string> LoadCarrierData(const std::string& directory, const std::string& language, const std::string& country_code, const std::string& separator = "|") {
    std::string file_path = directory + "/" + language + "/" + country_code + ".txt";

    if (!FileExists(file_path)) {
        return {};
    }

    std::ifstream file(file_path);
    if (!file.is_open()) {
        return {};
    }

    std::map<std::string, std::string> carrier_data;
    std::string line;

    while (std::getline(file, line)) {
        size_t separator_pos = line.find(separator);
        if (separator_pos != std::string::npos) {
            std::string phone_prefix = line.substr(0, separator_pos);
            std::string carrier_name = line.substr(separator_pos + separator.size());
            carrier_data[phone_prefix] = carrier_name;
        }
    }

    return carrier_data;
}

std::string GetCarrierForNumber(const std::string& phone_number, const std::map<std::string, std::string>& carrier_data) {
    for (const auto& entry : carrier_data) {
        if (phone_number.find(entry.first) == 0) {
            return entry.second;
        }
    }
    return "";
}

static bool parse_phone_number(const char* phone_number, const char* region, PhoneNumber& number) {
    return phone_util->Parse(phone_number, region, &number) == PhoneNumberUtil::NO_PARSING_ERROR;
}

extern "C" int parse(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_optstring(L, 2, "us");
    const char* language = luaL_optstring(L, 3, "en");
    const char* country = luaL_optstring(L, 4, "US");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        lua_newtable(L);

        if (number.has_country_code()) {
            lua_pushstring(L, "country_code");
            lua_pushinteger(L, number.country_code());
            lua_settable(L, -3);
        }

        if (number.has_raw_input()) {
            lua_pushstring(L, "raw_input");
            lua_pushstring(L, number.raw_input().c_str());
            lua_settable(L, -3);
        }

        std::string international_format;
        phone_util->Format(number, PhoneNumberUtil::INTERNATIONAL, &international_format);
        lua_pushstring(L, international_format.c_str());
        lua_setfield(L, -2, "INTERNATIONAL");

        std::string e164_format;
        phone_util->Format(number, PhoneNumberUtil::E164, &e164_format);
        lua_pushstring(L, e164_format.c_str());
        lua_setfield(L, -2, "E164");

        std::string national_format;
        phone_util->Format(number, PhoneNumberUtil::NATIONAL, &national_format);
        lua_pushstring(L, national_format.c_str());
        lua_setfield(L, -2, "NATIONAL");

        std::string rfc3966_format;
        phone_util->Format(number, PhoneNumberUtil::RFC3966, &rfc3966_format);
        lua_pushstring(L, rfc3966_format.c_str());
        lua_setfield(L, -2, "RFC3966");

        int number_type = phone_util->GetNumberType(number);
        switch (number_type) {
            case PhoneNumberUtil::FIXED_LINE:
                lua_pushstring(L, "FIXED_LINE");
                break;
            case PhoneNumberUtil::MOBILE:
                lua_pushstring(L, "MOBILE");
                break;
            case PhoneNumberUtil::FIXED_LINE_OR_MOBILE:
                lua_pushstring(L, "FIXED_LINE_OR_MOBILE");
                break;
            case PhoneNumberUtil::TOLL_FREE:
                lua_pushstring(L, "TOLL_FREE");
                break;
            case PhoneNumberUtil::PREMIUM_RATE:
                lua_pushstring(L, "PREMIUM_RATE");
                break;
            case PhoneNumberUtil::SHARED_COST:
                lua_pushstring(L, "SHARED_COST");
                break;
            case PhoneNumberUtil::VOIP:
                lua_pushstring(L, "VOIP");
                break;
            case PhoneNumberUtil::PERSONAL_NUMBER:
                lua_pushstring(L, "PERSONAL_NUMBER");
                break;
            case PhoneNumberUtil::PAGER:
                lua_pushstring(L, "PAGER");
                break;
            case PhoneNumberUtil::UAN:
                lua_pushstring(L, "UAN");
                break;
            case PhoneNumberUtil::VOICEMAIL:
                lua_pushstring(L, "VOICEMAIL");
                break;
            default:
                lua_pushstring(L, "UNKNOWN");
                break;
        }
        lua_setfield(L, -2, "type");

        std::string region_code;
        phone_util->GetRegionCodeForNumber(number, &region_code);
        lua_pushstring(L, region_code.c_str());
        lua_setfield(L, -2, "region");

        std::string location = geocoder.GetDescriptionForNumber(number, icu::Locale(language, country));
        lua_pushstring(L, location.c_str());
        lua_setfield(L, -2, "location");

        bool is_valid = phone_util->IsValidNumber(number);
        lua_pushboolean(L, is_valid);
        lua_setfield(L, -2, "is_valid");

        bool is_possible = phone_util->IsPossibleNumber(number);
        lua_pushboolean(L, is_possible);
        lua_setfield(L, -2, "is_possible");

        return 1;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

extern "C" int format(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_checkstring(L, 2);
    const char* format_type_str = luaL_checkstring(L, 3);

    PhoneNumberUtil::PhoneNumberFormat format_type;

    if (strcmp(format_type_str, "E164") == 0) {
        format_type = PhoneNumberUtil::E164;
    } else if (strcmp(format_type_str, "INTERNATIONAL") == 0) {
        format_type = PhoneNumberUtil::INTERNATIONAL;
    } else if (strcmp(format_type_str, "NATIONAL") == 0) {
        format_type = PhoneNumberUtil::NATIONAL;
    } else if (strcmp(format_type_str, "RFC3966") == 0) {
        format_type = PhoneNumberUtil::RFC3966;
    } else {
        lua_pushnil(L);
        return 1;
    }

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        std::string formatted_number;
        phone_util->Format(number, format_type, &formatted_number);
        lua_pushstring(L, formatted_number.c_str());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

extern "C" int get_number_type(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_optstring(L, 2, "us");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        PhoneNumberUtil::PhoneNumberType number_type = phone_util->GetNumberType(number);

        switch (number_type) {
            case PhoneNumberUtil::FIXED_LINE:
                lua_pushstring(L, "FIXED_LINE");
                break;
            case PhoneNumberUtil::MOBILE:
                lua_pushstring(L, "MOBILE");
                break;
            case PhoneNumberUtil::FIXED_LINE_OR_MOBILE:
                lua_pushstring(L, "FIXED_LINE_OR_MOBILE");
                break;
            case PhoneNumberUtil::TOLL_FREE:
                lua_pushstring(L, "TOLL_FREE");
                break;
            case PhoneNumberUtil::PREMIUM_RATE:
                lua_pushstring(L, "PREMIUM_RATE");
                break;
            case PhoneNumberUtil::SHARED_COST:
                lua_pushstring(L, "SHARED_COST");
                break;
            case PhoneNumberUtil::VOIP:
                lua_pushstring(L, "VOIP");
                break;
            case PhoneNumberUtil::PERSONAL_NUMBER:
                lua_pushstring(L, "PERSONAL_NUMBER");
                break;
            case PhoneNumberUtil::PAGER:
                lua_pushstring(L, "PAGER");
                break;
            case PhoneNumberUtil::UAN:
                lua_pushstring(L, "UAN");
                break;
            case PhoneNumberUtil::VOICEMAIL:
                lua_pushstring(L, "VOICEMAIL");
                break;
            default:
                lua_pushstring(L, "UNKNOWN");
                break;
        }
    } else {
        lua_pushnil(L);
    }

    return 1;
}

extern "C" int is_possible_number(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_optstring(L, 2, "us");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        bool possible = phone_util->IsPossibleNumber(number);
        lua_pushboolean(L, possible);
    } else {
        lua_pushboolean(L, 0);
    }

    return 1;
}

extern "C" int is_valid_number(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_optstring(L, 2, "us");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        bool valid = phone_util->IsValidNumber(number);
        lua_pushboolean(L, valid);
    } else {
        lua_pushboolean(L, 0);
    }

    return 1;
}

extern "C" int get_region(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_optstring(L, 2, "us");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        std::string region_code;
        phone_util->GetRegionCodeForNumber(number, &region_code);
        lua_pushstring(L, region_code.c_str());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

extern "C" int get_location(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_checkstring(L, 2);
    const char* language = luaL_optstring(L, 3, "en");
    const char* country = luaL_optstring(L, 4, "US");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        std::string location = geocoder.GetDescriptionForNumber(number, icu::Locale(language, country));
        lua_pushstring(L, location.c_str());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

extern "C" int as_you_type_formatter(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_optstring(L, 2, "us");

    std::unique_ptr<AsYouTypeFormatter> formatter(phone_util->GetAsYouTypeFormatter(region));

    std::string formatted_number;
    std::string current_output;

    for (size_t i = 0; phone_number[i] != '\0'; ++i) {
        formatter->InputDigit(static_cast<char32>(phone_number[i]), &current_output);
    }

    lua_pushstring(L, current_output.c_str());
    return 1;
}

extern "C" int get_carrier(lua_State* L) {
    const char* phone_number = luaL_checkstring(L, 1);
    const char* region = luaL_checkstring(L, 2);
    const char* directory = luaL_checkstring(L, 3);
    const char* language = luaL_optstring(L, 4, "en");
    const char* separator = luaL_optstring(L, 5, "|");

    PhoneNumber number;
    if (parse_phone_number(phone_number, region, number)) {
        std::string country_code = "";

        if (number.has_country_code()) {
            country_code = std::to_string(number.country_code());
        }

        if (country_code != "") {
            std::string e164_format;
            phone_util->Format(number, PhoneNumberUtil::E164, &e164_format);
            std::string _phone_number = e164_format.c_str();
            _phone_number = _phone_number.substr(1);

            std::map<std::string, std::string> carrier_data = LoadCarrierData(directory, language, country_code, separator);
            std::string carrier = GetCarrierForNumber(_phone_number, carrier_data);
            lua_pushstring(L, carrier.c_str());
        } else {
            lua_pushstring(L, "");
        }

    } else {
        lua_pushstring(L, "");
    }

    return 1;
}

static const luaL_Reg lib_phone_number[] = {
    {"parse", parse},
    {"format", format},
    {"get_number_type", get_number_type},
    {"is_possible_number", is_possible_number},
    {"is_valid_number", is_valid_number},
    {"get_region", get_region},
    {"get_location", get_location},
    {"as_you_type_formatter", as_you_type_formatter},
    {"get_carrier", get_carrier},
    {NULL, NULL}
};

extern "C" int luaopen_libphonenumber(lua_State* L) {
#if LUA_VERSION_NUM == 501
    luaL_register(L, "libphonenumber", lib_phone_number);
#else
    luaL_newlib(L, lib_phone_number);
#endif
    return 1;
}

#include "Configuration.h"

#include <iostream>
#include "tao/json.hpp"
#include "tao/json/contrib/traits.hpp"

Configuration::Configuration(std::string f)
{
    const tao::json::value parsedJson = tao::json::from_file(f.c_str());
    std::cout << parsedJson << std::endl;
}

Configuration::~Configuration()
{
}

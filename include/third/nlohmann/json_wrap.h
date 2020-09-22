//
// Created by Administrator on 2018/5/29.
//

#pragma once

#include <iostream>
#include <sstream>
#include "svrlib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace svrlib;

// 解析json对象
bool ParseJsonFromBuff(json& jvalue, const uint8_t* data, size_t size);

// 解析json对象
bool ParseJsonFromString(json& jvalue, const string& data);



//
// Created by Administrator on 2018/5/30.
//

#include "nlohmann/json_wrap.h"


// 解析json对象
bool ParseJsonFromBuff(json& jvalue, const uint8_t* data, size_t size)
{
    try
    {
        jvalue = json::parse(data, data + size);
    }
    catch (const json::parse_error& e)
    {
        LOG_ERROR("parse errors {},exception:{},byte position of error:{}",e.what(),e.id,e.byte);
        jvalue.clear();
        return false;
    }
    catch (const json::exception& e)
    {
        LOG_ERROR("parse errors {},exception:{}",e.what(),e.id);
        jvalue.clear();
        return false;
    }

    return true;
}
// 解析json对象
bool ParseJsonFromString(json& jvalue, const string& data)
{
    try
    {
        jvalue = json::parse(data);
    }
    catch (const json::parse_error& e)
    {
        LOG_ERROR("parse errors {},exception:{},byte position of error:{}",e.what(),e.id,e.byte);
        jvalue.clear();
        return false;
    }
    catch (const json::exception& e)
    {
        LOG_ERROR("parse errors {},exception:{}",e.what(),e.id);
        jvalue.clear();
        return false;
    }

    return true;
}





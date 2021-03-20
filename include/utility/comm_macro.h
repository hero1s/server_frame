
#pragma once

#include <stdint.h>
#include <thread>

#include "spdlog/spdlog.h"

//@brief  条件检测
#define CHECK_RET(EXPR, RET)\
do{\
  if (!(EXPR)){return (RET);}\
}while(0);

//@brief  条件检测
#define CHECK_VOID(EXPR)\
do{\
  if (!(EXPR)){return;}\
}while(0);

//@brief  条件检测
#define CHECK_LOG_RET(EXPR, RET, FMT, ARG...)\
do{\
  if (!(EXPR)){LOG_ERROR(FMT, ##ARG);return (RET);}\
}while(0);

//@brief  条件检测
#define CHECK_LOG_VOID(EXPR, FMT, ARG...)\
do{\
  if (!(EXPR)){LOG_ERROR(FMT, ##ARG);return;}\
}while(0);




//@brief  ASSERT
#define ASSERT_RET(EXPR, RET)\
do{\
  if (!(EXPR)){LOG_ERROR("{}", #EXPR);assert(false);return (RET);}\
}while(0);

//@brief  ASSERT
#define ASSERT_VOID(EXPR)\
do{\
  if (!(EXPR)){LOG_ERROR("{}", #EXPR);assert(false);return;}\
}while(0);

//@brief  ASSERT
#define ASSERT(EXPR)\
do{\
  if (!(EXPR)){LOG_ERROR("{}", #EXPR);assert(false);}\
}while(0);

//@brief  ASSERT
#define ASSERT_LOG_RET(EXPR, RET, FMT, ARG...)\
do{\
  if (!(EXPR)){LOG_ERROR(FMT, ##ARG);assert(false);return (RET);}\
}while(0);

//@brief  ASSERT
#define ASSERT_LOG_VOID(EXPR, FMT, ARG...)\
do{\
  if (!(EXPR)){LOG_ERROR(FMT, ##ARG);assert(false);return;}\
}while(0);

//@brief  ASSERT
#define ASSERT_LOG(EXPR, FMT, ARG...)\
do{\
  if (!(EXPR)){LOG_ERROR(FMT, ##ARG);assert(false);}\
}while(0);


//@brief	结构体构造函数
#define STRUCT_ZERO(TYPE)\
    TYPE(){memset(this, 0, sizeof(*this));}

#define ZeroMemory(Destination, Length) memset((Destination),0,(Length))

//@brief	无符号数据加减
static inline uint32_t U32Sub(uint32_t a, uint32_t b)
{
	if (a > b)
	{
		return a - b;
	}
	else
	{
		return 0;
	}
}

static inline uint32_t U32Add(uint32_t a, uint32_t b)
{
	if (0xFFFFFFFF - a > b)
	{
		return a + b;
	}
	else
	{
		return 0xFFFFFFFF;
	}
}

//sol2 lua 调用
#ifndef SOL_CALL_LUA
#define SOL_CALL_LUA(lua)\
try{\
    lua;\
}\
catch (sol::error& e)\
{\
    LOG_ERROR("sol error:{} ", e.what());\
}
#endif

//spdlog
#ifndef LOG_TRACE
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define LOG_TRACE(x,...) spdlog::get("log")->trace("[{}:{}][{}] " x,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define LOG_DEBUG(x,...) spdlog::get("log")->debug("[{}:{}][{}] " x,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(x,...)  spdlog::get("log")->info("[{}:{}][{}] " x,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(x,...) spdlog::get("log")->error("[{}:{}][{}] " x,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define LOG_CRITIC(x,...) spdlog::get("log")->critical("[{}:{}][{}] " x,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(x,...) spdlog::get("log")->warn("[{}:{}][{}] " x,__FILENAME__,__LINE__,__FUNCTION__, ##__VA_ARGS__)

#endif

#ifndef DUMP_PROTO_MSG_INFO

#define DUMP_PROTO_MSG_INFO(msg) \
do { \
    LOG_INFO("Dump {}{}{}", (msg).GetTypeName(), ":\n", (msg).DebugString()); \
} while (0)

#define DUMP_PROTO_MSG_WARNING(msg) \
do { \
    LOG_WARNING("Dump {}{}{}", (msg).GetTypeName(), ":\n", (msg).DebugString()); \
} while (0)

#define DUMP_PROTO_MSG_ERROR(msg) \
do { \
    LOG_ERROR("Dump {}{}{}", (msg).GetTypeName(), ":\n", (msg).DebugString()); \
} while (0)

#endif

#ifndef thread_sleep
#define thread_sleep(x)  std::this_thread::sleep_for(std::chrono::milliseconds(x));
#endif






#pragma once

#include "thread/rwlock.hpp"
#include <string>
#include <unordered_map>


//#define USE_PROFILE                     //是否启用性能分析

#define GETTHREADID() (syscall(__NR_gettid))

class ProfileManager;

class ProfileNode {
public:
    ProfileNode(const char *node_name, ProfileNode *parent_node, ProfileManager *mgr);

    virtual ~ProfileNode(void);

    //@brief    根据名称得到子节点
    ProfileNode *getSubNode(const char *node_name);

    //@brief    根据索引得到子节点
    ProfileNode *getSubNode(int index);

    //@brief    得到父节点
    ProfileNode *getParentNode(void) { return m_parent_node; };

    //@brief    取得兄弟节点
    ProfileNode *getSiblingNode(void) { return m_sibling_node; };

    //@brief    取得子节点
    ProfileNode *getChildNode(void) { return m_child_node; };

    //@brief    重置
    void reset(void);

    //@brief    当前进入本节点的测试时被调用
    void enter(void);

    //@breif    退出本节点的测试时调用
    bool leave(void);

    //@brief    取得本节点的名称
    const char *getNodeName(void) const { return m_node_name.c_str(); };

    //@brief    取得总共被调用的次数
    int getTotalCalls(void) const { return m_total_calls; };

    //@brief    取得峰值时间
    float getPeakValue(void) const { return m_peak_value; };

    //@brief    取得占用时间
    float getTotalTime(void) const { return m_total_time; };

    //@brief    取得本节点在父节点中占用的时间百分比
    float getPercentInParent(void) const { return m_percent_in_parent; };

    //@brief    得取当前线程ID
    uint32_t getCurThreadID() const { return m_cur_threadid; };

    //@brief    设置当前线程ID
    void setCurThreadID(uint32_t threadid) { m_cur_threadid = threadid; };

    //@brief    保存统计数据
    bool saveData(void *file, uint32_t layer/*= 0*/);

    //@brief    指定统计信息
    void statInfo(uint32_t flag);

private:
    uint32_t m_cur_threadid;          //当前线程ID
    uint32_t m_total_calls;           //总调用次数
    int m_recursion_counter;          //递归调用计数
    float m_total_time;               //总时间
    float m_peak_value;               //此节点执行时间峰值
    float m_percent_in_parent;        //占用父节点的时间百分比
    int64_t m_start_time;             //开始时间

    std::string m_node_name;        //节点名称
    ProfileNode *m_parent_node;     //父节点
    ProfileNode *m_child_node;      //子节点
    ProfileNode *m_sibling_node;    //兄弟节点
    ProfileManager *m_mgr;

    friend class ProfileManager;
};


class ProfileManager {
public:
    enum {
        flag_stat_percentinparent = 1,
    };

    ProfileManager(void);

    virtual ~ProfileManager(void);

    //@brief    初始化
    void init();

    //@brief    结束
    void shutdown();

    //@brief    开始统计一下性能测试节点，并将其设置为当前节点
    void startProfile(const char *node_name);

    //@brief    结束当前节点的性能统计
    void stopProfile(const char *node_name);

    //@brief    设置统计信息
    void statInfo(uint32_t flag /*= flag_stat_percentinparent */);

    //@brief    重置
    void reset(void);

    //@brief    增加帧数，应该在第帧中调用
    void increaseFrameCount(void) { ++m_frame_counter; };

    //@brief    取得经过的帧数
    uint32_t getFrameCount(void) { return m_frame_counter; };

    //@brief    取得根节点
    ProfileNode *getRootNode(void) { return m_root_node; };

    //@brief    设置性能统计类型
    void setProfileType(uint8_t profile_type) { m_profile_type = profile_type; };

    //@brief    得到性能统计类型
    uint8_t getProfileType(void) { return m_profile_type; };

    //@brief    得到重置以来的时间
    float getTimeSinceReset();

    //@brief    保存数据到指定文件
    bool saveData(const char *file_name);

    //@brief    保存数据到默认文件
    bool saveData(const char *servername, uint32_t serverid, uint32_t tid = 0);

private:
    ProfileNode *m_root_node;            //根节点
    ProfileNode *m_cur_node;             //当前节点
    uint8_t m_profile_type;              //性能统计类型
    uint32_t m_frame_counter;            //帧数
    int64_t m_reset_time;                //开始计时时间
};


class GlobalProfileManager {
public:
    GlobalProfileManager();

    ~GlobalProfileManager();

    ProfileManager *getMgr();

    void init();

    void shutdown(const char *servername, uint32_t serverid);

    std::unordered_map<uint32_t, ProfileManager *> mgr_map;
    svrlib::rwlock locker;
};

GlobalProfileManager &g_GetGMgr();


#ifdef USE_PROFILE
#define PROFILE_BEGIN(NODE_NAME)\
    do{\
        g_GetGMgr().getMgr()->startProfile(NODE_NAME);\
    }while(0);

#define PROFILE_END(NODE_NAME)\
    do{\
        g_GetGMgr().getMgr()->stopProfile(NODE_NAME);\
    }while(0);

#define PROFILE_INIT()\
    do{\
        g_GetGMgr().init();\
    }while(0);

#define PROFILE_SHUTDOWN(SERVERNAME, SERVERID)\
    do{\
        g_GetGMgr().shutdown(SERVERNAME, SERVERID);\
    }while(0);
#else
#define PROFILE_BEGIN(NODE_NAME)
#define PROFILE_END(NODE_NAME)

#define PROFILE_INIT()
#define PROFILE_SHUTDOWN(SERVERNAME, SERVERID)
#endif


class AutoProfile {
    const char *node_name_;
public:
    explicit AutoProfile(const char *node_name) : node_name_(node_name) {
        PROFILE_BEGIN(node_name_);
    }

    ~AutoProfile() {
        PROFILE_END(node_name_);
    }
};

#ifdef USE_PROFILE
#define AUTOPROFILE(NODE_NAME)\
    auto AutoPP = AutoProfile(NODE_NAME);
#else
#define AUTOPROFILE(NODE_NAME)
#endif

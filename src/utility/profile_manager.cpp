/**
 * @author	toney
 * @date	2019年04月26日
 * @brief	性能分析统计
 */


#include "utility/profile_manager.h"
#include "utility/basic_types.h"
#include "utility/comm_macro.h"
#include "time/time.hpp"
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#define MAX_PATH 255

enum PROFILETYPE {
    PROFILETYPE_NONE = 0,           //无统计
    PROFILETYPE_SIMPLE = 1,         //简易
    PROFILETYPE_PRECISION = 2,      //精确
};

inline void profileGetTime(int64_t *time) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *time = tv.tv_sec * 1000000 + tv.tv_usec;
}

inline bool profileGetAppPath(char *path, int len) {
    char sysfile[MAX_PATH] = "/proc/self/exe";
    return (-1 != readlink(sysfile, path, len));
}

inline bool createLogPath(char *path, int len) {

    char app_path[MAX_PATH] = {'\0'};
    if (!profileGetAppPath(app_path, MAX_PATH)) {
        LOG_ERROR("create log path fail:{}",path);
        return false;
    }
    std::string str_path(app_path);
    sprintf(path, "%s/Analyze/", str_path.substr(0, str_path.find_last_of('/')).c_str());

    if (access(path, F_OK) != 0) {
        if (mkdir(path, 0755) == -1) {
            LOG_ERROR("no access mode:{}",path);
            return false;
        }
    }
    return true;
}


ProfileNode::ProfileNode(const char *node_name, ProfileNode *parent_node, ProfileManager *mgr)
        : m_cur_threadid(0), m_total_calls(0), m_recursion_counter(0), m_total_time(0.0f), m_peak_value(0.0f),
          m_percent_in_parent(0.0f), m_start_time(0), m_node_name(node_name), m_parent_node(parent_node),
          m_child_node(NULL), m_sibling_node(NULL), m_mgr(mgr) {
    reset();
}

ProfileNode::~ProfileNode() {
    if (m_child_node) {
        SAFE_DELETE(m_child_node);
    }
    if (m_sibling_node) {
        SAFE_DELETE(m_sibling_node);
    }
}

ProfileNode *ProfileNode::getSubNode(const char *node_name) {
    if (NULL == node_name) {
        return NULL;
    }
    ProfileNode *temp_child = m_child_node;
    while (temp_child) {
        if (0 == strcmp(node_name, temp_child->getNodeName())) {
            return temp_child;
        }
        temp_child = temp_child->getSiblingNode();
    }
    //如果有没找到则新建
    ProfileNode *new_node = new ProfileNode(node_name, this, this->m_mgr);
    new_node->m_sibling_node = m_child_node;
    new_node->setCurThreadID(m_mgr->getRootNode()->getCurThreadID());
    m_child_node = new_node;
    LOG_DEBUG(" {} add new anazy node:{} ",getNodeName(),node_name);
    return new_node;
}

ProfileNode *ProfileNode::getSubNode(int index) {
    ProfileNode *temp_child = m_child_node;
    while (temp_child && index--) {
        temp_child = temp_child->getSiblingNode();
    }
    return temp_child;
}

void ProfileNode::reset(void) {
    m_total_calls = 0;
    m_total_time = 0.0f;
    m_peak_value = 0.0f;
    if (m_child_node) {
        m_child_node->reset();
    }
    if (m_sibling_node) {
        m_sibling_node->reset();
    }
}

void ProfileNode::enter(void) {
    m_total_calls++;
    if (0 == m_recursion_counter++) {
        switch (m_mgr->getProfileType()) {
            case PROFILETYPE_NONE: {
                break;
            }
            case PROFILETYPE_SIMPLE: {
                profileGetTime(&m_start_time);
                break;
            }
            case PROFILETYPE_PRECISION: {
                profileGetTime(&m_start_time);
                break;
            }
            default:
                break;
        }
    }
}


bool ProfileNode::leave(void) {
    if (0 == --m_recursion_counter && 0 != m_total_calls) {
        switch (m_mgr->getProfileType()) {
            case PROFILETYPE_NONE: {
                break;
            }
            case PROFILETYPE_SIMPLE: {
                int64_t cur_time;
                profileGetTime(&cur_time);
                float real_time = (float) ((cur_time - m_start_time) / 1000000.0f);
                if (real_time > m_peak_value) {
                    m_peak_value = real_time;
                }
                m_total_time += real_time;
                break;
            }
            case PROFILETYPE_PRECISION: {
                int64_t cur_time;
                profileGetTime(&cur_time);
                float real_time = (float) ((cur_time - m_start_time) / 1000000.0f);
                if (real_time > m_peak_value) {
                    m_peak_value = real_time;
                }
                m_total_time += real_time;
                break;
            }
            default:
                break;
        }
    }
    return 0 == m_recursion_counter;
}


bool ProfileNode::saveData(void *f, uint32_t layer/*= 0*/) {
    if (NULL == f || layer >= 1024) {
        LOG_ERROR("saveData f is null or layer >= 1024");
        return false;
    }

    char space[1024] = {'\0'};
    for (uint32_t i = 0; i < layer; ++i) {
        space[i] = '\t';
    }

    char text[1024] = {'\0'};
    sprintf(text, "[%u]Name:%s\tPercent:%0.3f%%\tTotalTime:%.6f(s)\tAvgTime:%.6f(s)\tMaxTime:%.6f(s)\tCount:%u\t\n",
            layer, getNodeName(), getPercentInParent() * 100.0f, getTotalTime(), getTotalTime() / getTotalCalls(),
            getPeakValue(), getTotalCalls());

    char enum_begin[256] = {'\0'};
    sprintf(enum_begin, "{\n");

    char enum_end[256] = {'\n'};
    sprintf(enum_end, "}\n");

    FILE *file = static_cast<FILE *>(f);
    fwrite(space, strlen(space), 1, file);
    fwrite(text, strlen(text), 1, file);

    ProfileNode *node = getChildNode();
    if (NULL != node) {
        fwrite(space, strlen(space), 1, file);
        fwrite(enum_begin, strlen(enum_begin), 1, file);

        while (NULL != node) {
            node->saveData(file, layer + 1);
            node = node->getSiblingNode();
        }

        fwrite(space, strlen(space), 1, file);
        fwrite(enum_end, strlen(enum_end), 1, file);
    }

    return true;
}


void ProfileNode::statInfo(uint32_t flag) {
    ProfileNode *child_node = m_child_node;
    while (NULL != child_node) {
        if (flag & ProfileManager::flag_stat_percentinparent && getTotalTime() > 0.000000001f) {
            child_node->m_percent_in_parent = child_node->getTotalTime() / getTotalTime();
        }
        child_node->statInfo(flag);
        child_node = child_node->getSiblingNode();
    }
}


ProfileManager::ProfileManager(void)
        : m_root_node(NULL), m_cur_node(NULL), m_profile_type(PROFILETYPE_PRECISION), m_frame_counter(0),
          m_reset_time(0) {

}

ProfileManager::~ProfileManager(void) {
}

void ProfileManager::init() {
    if (NULL == m_root_node) {
        m_root_node = new ProfileNode("Root", NULL, this);
        reset();
        m_cur_node = m_root_node;
        m_cur_node->setCurThreadID(GETTHREADID());
        m_cur_node->enter();
    }
}

void ProfileManager::shutdown() {
    if (NULL != m_root_node) {
        m_root_node->leave();
        SAFE_DELETE(m_root_node);
        m_root_node = NULL;
        m_cur_node = NULL;
    }
}

void ProfileManager::startProfile(const char *node_name) {
    if (NULL == node_name || NULL == m_cur_node) {
        return;
    }
    uint32_t cur_threadid = GETTHREADID();
    if (cur_threadid != m_cur_node->getCurThreadID()) {
        return;
    }
    if (0 != strcmp(node_name, m_cur_node->getNodeName())) {
        m_cur_node = m_cur_node->getSubNode(node_name);
    }
    if (NULL != m_cur_node) {
        m_cur_node->enter();
    }
}

void ProfileManager::stopProfile(const char *node_name) {
    if (NULL == m_cur_node || NULL == node_name) {
        return;
    }
    uint32_t cur_threadid = GETTHREADID();
    if (cur_threadid != m_cur_node->getCurThreadID()) {
        return;
    }
    if (0 != strcmp(node_name, m_cur_node->getNodeName())) {
        LOG_ERROR("node:{},curnode:{}",node_name,m_cur_node->getNodeName());
        return;
    }
    if (m_cur_node->leave()) {
        m_cur_node = m_cur_node->getParentNode();
    }
}

void ProfileManager::statInfo(uint32_t flag /*= flag_stat_percentinparent */) {
    if (NULL == m_root_node) {
        return;
    }
    m_root_node->m_total_time = getTimeSinceReset();
    m_root_node->statInfo(flag);
}

void ProfileManager::reset(void) {
    if (NULL == m_root_node) {
        return;
    }
    m_root_node->reset();
    m_frame_counter = 0;
    profileGetTime(&m_reset_time);
}

float ProfileManager::getTimeSinceReset() {
    int64_t time;
    profileGetTime(&time);
    return (float) ((time - m_reset_time) / 1000000.0f);
}


bool ProfileManager::saveData(const char *file_name) {
    if (NULL == m_root_node || NULL == file_name) {
        LOG_ERROR("root node is null or file_:{}",file_name);
        return false;
    }
    ProfileManager::setProfileType(PROFILETYPE_NONE);
    statInfo(flag_stat_percentinparent);

    FILE *file = NULL;
    if (NULL == (file = fopen(file_name, "a+"))) {
        LOG_ERROR("open file fail:{}",file_name);
        return false;
    }

    m_root_node->saveData((void *) file, 0);
    fclose(file);
    file = NULL;
    return true;
}

bool ProfileManager::saveData(const char *servername, uint32_t serverid, uint32_t tid) {
    char app_path[MAX_PATH-100] = {'\0'};
    if (!createLogPath(app_path, MAX_PATH)) {
        return false;
    }
    char data_path[MAX_PATH] = {'\0'};
    if (tid == 0) {
        tid = GETTHREADID();
    }
    sprintf(data_path, "%s%s_%s_%d_%d.log", app_path, svrlib::time::time_format(svrlib::time::second()), servername, serverid, tid);
    return saveData(data_path);
}

GlobalProfileManager::GlobalProfileManager() {
}

GlobalProfileManager::~GlobalProfileManager() {
    for (auto &it : mgr_map) {
        ProfileManager *mgr = it.second;
        SAFE_DELETE(mgr);
    }
    mgr_map.clear();
}

ProfileManager *GlobalProfileManager::getMgr() {
    uint32_t tid = GETTHREADID();
    locker.lock_shared();
    auto it = mgr_map.find(tid);
    locker.unlock_shared();
    if (it != mgr_map.end()) {
        return it->second;
    } else {
        ProfileManager *mgr = new ProfileManager();
        locker.lock();
        mgr_map[tid] = mgr;
        mgr->init();
        locker.unlock();
        return mgr;
    }
}

void GlobalProfileManager::init(){
    getMgr();
}

void GlobalProfileManager::shutdown(const char *servername, uint32_t serverid) {
    LOG_DEBUG("profile Analyze shutdown and save file:{}.{}",servername,serverid);
    locker.lock();
    auto it = mgr_map.begin();
    for (; it != mgr_map.end(); ++it) {
        ProfileManager *mgr = it->second;
        if (mgr) {
            mgr->saveData(servername, serverid, it->first);
            mgr->shutdown();
        }
    }
    locker.unlock();
}

GlobalProfileManager &g_GetGMgr() {
    static GlobalProfileManager g_GlobalMgr;
    return g_GlobalMgr;
}


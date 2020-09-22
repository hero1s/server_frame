/*=============================================================================
#
#     FileName: test.cpp
#         Desc: 
#
#       Author: dantezhu
#        Email: zny2008@gmail.com
#     HomePage: http://www.vimer.cn
#
#      Created: 2011-03-12 12:58:21
#      Version: 0.0.1
#      History:
#               0.0.1 | dantezhu | 2011-03-12 12:58:21 | initialization
#
=============================================================================*/
/*
CREATE TABLE `tb_test` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(32) default "",
  `sex` int(11) default 0,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
);
*/
/*
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <gtest/gtest.h>

#include "dbmysql/sql_join.h"
#include "dbmysql/mysql_wrapper.h"

using namespace std;

#ifndef foreach
#define foreach(container,it) \
    for(typeof((container).begin()) it = (container).begin();it!=(container).end();++it)
#endif

CMYSQLWrapper g_client;

string g_name = "good";
int g_sex = 1;

string g_name_up = "update";
int g_sex_up = 2;

int clear_data()
{
    stringstream ss;
    ss << "delete from tb_test";

    int affectRowsNum;
    int ret = g_client.Query(ss.str().c_str(),affectRowsNum);
    return ret;
}

TEST(mysql_wrapper_easy, insert)
{
    clear_data();
    stringstream ss;
    ss 
        << "insert into tb_test(name,sex) values('"
        << g_client.EscStr(g_name.c_str())
        << "',"
        << g_sex
        << ");";

    int affectRowsNum;
    int ret = g_client.Query(ss.str().c_str(), affectRowsNum);
    ASSERT_EQ(ret, 0) << g_client.GetErrMsg();

    EXPECT_GE(affectRowsNum,0) << g_client.GetErrMsg();
}
TEST(mysql_wrapper_easy, update)
{
    stringstream ss;
    ss 
        << "update tb_test set sex="
        << g_sex_up
        << ", name='"
        << g_name_up
        << "' "
        << "where name='"
        << g_name
        <<"';";
    int affectRowsNum;
    int ret = g_client.Query(ss.str().c_str(),affectRowsNum);
    ASSERT_EQ(ret, 0) << g_client.GetErrMsg();

    EXPECT_GE(affectRowsNum,0) << g_client.GetErrMsg();
}
TEST(mysql_wrapper_easy, select)
{
    vector<map<string,MYSQLValue> > vecData;
    string sql = "select * from tb_test where name = '"+g_name_up+"'";
    int ret = g_client.Query(sql.c_str(),vecData);
    ASSERT_EQ(ret, 0) << g_client.GetErrMsg();

    foreach(vecData, it_vec)
    {   
        foreach(*it_vec, it_map)
        {   
            cout << it_map->first << ",";
            if (it_map->first == "sex")
            {
                cout << it_map->second.as<uint32_t>();
            }
            else
            {
                cout << it_map->second.data();
            }
            cout << "," << it_map->second.size() << endl;
        }   
    }
}


TEST(mysql_wrapper_join, insert)
{
    clear_data();

    SQLJoin sql_join;
    sql_join 
        << SQLPair("name", g_client.EscStr(g_name.c_str()))
        << SQLPair("sex", g_sex);

    stringstream ss;
    ss 
        << "insert into tb_test("
        << sql_join.keys()
        << ") values("
        << sql_join.values()
        << ")";

    int affectRowsNum;
    int ret = g_client.Query(ss.str().c_str(), affectRowsNum);
    ASSERT_EQ(ret, 0) << g_client.GetErrMsg();

    EXPECT_GE(affectRowsNum,0) << g_client.GetErrMsg();
}
TEST(mysql_wrapper_join, update)
{
    SQLJoin sql_join;
    sql_join 
        << SQLPair("name", g_name_up)
        << SQLPair("sex", g_sex_up);

    stringstream ss;
    ss 
        << "update tb_test set "
        << sql_join.pairs()
        << " where name='"
        << g_name
        <<"';";
    int affectRowsNum;
    int ret = g_client.Query(ss.str().c_str(),affectRowsNum);
    ASSERT_EQ(ret, 0) << g_client.GetErrMsg();

    EXPECT_GE(affectRowsNum,0) << g_client.GetErrMsg();
}

TEST(mysql_wrapper_join, select)
{
    SQLJoin sql_join1;
    SQLJoin sql_join2;

    sql_join1
        << "id"
        << "name"
        << "sex";

    sql_join2
        << SQLPair("name",g_name_up)
        << SQLPair("sex",g_sex_up);

    stringstream ss;
    ss 
        << "select "
        << sql_join1.keys()
        << " from tb_test where "
        << sql_join2.pairs("and");

    vector<map<string,MYSQLValue> > vecData;
    int ret = g_client.Query(ss.str(),vecData);
    ASSERT_EQ(ret, 0) << g_client.GetErrMsg();

    foreach(vecData, it_vec)
    {   
        foreach(*it_vec, it_map)
        {   
            cout << it_map->first << ",";
            if (it_map->first == "sex")
            {
                cout << int(it_map->second.as<char>());
            }
            else
            {
                cout << it_map->second.data();
                //cout << it_map->second.as<char*>();
                //cout << it_map->second.as<const char*>();
            }
            cout << "," << it_map->second.size() << endl;
        }   
    }
}

TEST(mysql_value, null)
{
    map<string,MYSQLValue> mapData;
    cout << mapData[""].as<uint32_t>() << endl;
}

int main(int argc, char **argv)
{
    int ret = g_client.Init("10.6.207.119","dantezhu","dantezhu","soci",3306);
    //int ret = g_client.Init("localhost","dantezhu",NULL,"soci",3306);
    //int ret = g_client.Init("127.0.0.1","dantezhu",NULL,"soci");
    ret = g_client.Open();
    if (ret)
    {
        cout << ret << "," << g_client.GetErrMsg() << endl;
        return -1;
    }
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/

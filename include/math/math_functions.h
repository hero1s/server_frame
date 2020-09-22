
/***************************************************************
* 
***************************************************************/
#pragma once

#include "utility/basic_types.h"
#include "point2.h"
#include <math.h>
#include <vector>
#include <algorithm>

using namespace std;
namespace svrlib
{
/*************************************************************/
//#############################################################
//############################## 角度/弧度
//#############################################################
//--- 获得PI内的弧度
extern float getPiRadian(float fRadian);

//--- 获得2PI内的弧度
extern float get2PiRadian(float fRadian);

//--- 获得PI内的弧度
extern float getPiRadian(float m, float n);

//--- 获得2PI内的弧度
extern float get2PiRadian(float m, float n);

//--- 获得角度
extern float getPiAngle(float fAngle);

extern float get2PiAngle(float fAngle);

//--- 获得角度
extern float getPiAngle(float m, float n);

extern float get2PiAngle(float m, float n);

//--- 弧度转为角度
extern float getRadianToAngle(float fRadian);

//--- 角度转为弧度
extern float getAngleToRadian(float fAngle);

//--- 判断弧度是否在弧度范围内
extern bool inRadianRange(float f2PiRadian, float fRange, float fRadian);

//--- 判断角度是否在角度范围内
extern bool inAngleRange(float f2PiAngle, float fRange, float fAngle);

//--- 获得两点间的角度
extern float get2PiAngle(const _stPoint2F& a, const _stPoint2F& b);

//--- 获得两点间的弧度
extern float get2PiRadian(const _stPoint2F& a, const _stPoint2F& b);

//#############################################################
//############################## 点
//#############################################################
//------------------------------ 
//--- 获得两点距离
extern float getDistance(const _stPoint2F& a, const _stPoint2F& b);

extern int FastDistance(const _stPoint2I& a, const _stPoint2I& b);

//--- 获得点到线的距离(a和b是线段的两个端点， c是检测点)
extern float getPointToLineDistance(const _stPoint2F& a, const _stPoint2F& b, const _stPoint2F& p);

//--- 是否在范围内
extern bool inRange(const _stPoint2I& a, const _stPoint2I& b, int32_t iRange);

//---快速平方根
extern double FastSqrt(double d);

extern int FastSqrtInt(int d);

//---快速距离算法
extern int approx_distance2D(int dx, int dy);

extern int fastDistance2D(int dx, int dy);

/*--- 除法
@_molecular		=分子
@_denominator	=分母
@_floor			=向下取整
*/
extern uint32_t dDivisionUint32(uint64_t _molecular, uint32_t _denominator, bool _floor = true);

// 排列组合(注意控制组合结果值数量)
template<typename T>
void PrintCombRes(const vector<T>& t, vector<int>& vecInt, vector<vector<T> >& results)
{
	vector<T>   tmp;
	for (uint32_t k = 0; k < vecInt.size(); ++k)
	{
		if (vecInt[k] == 1)
		{
			tmp.push_back(t[k]);
		}
	}
	results.push_back(tmp);
}

// 从t中选c个对象,组合
template<typename T>
void Combination(const vector<T>& t, int c, vector<vector<T> >& results)
{
	//initial first combination like:1,1,0,0,0
	vector<int> vecInt(t.size(), 0);
	for (int    i = 0; i < c; ++i)
	{
		vecInt[i] = 1;
	}
	results.clear();
	PrintCombRes<T>(t, vecInt, results);
	for (int i = 0; i < (int) t.size() - 1; ++i)
	{
		if (vecInt[i] == 1 && vecInt[i + 1] == 0)
		{
			//1. first exchange 1 and 0 to 0 1
			std::swap(vecInt[i], vecInt[i + 1]);

			//2.move all 1 before vecInt[i] to left
			std::sort(vecInt.begin(), vecInt.begin() + i, [](T a, T b)
			{ return (a > b) ? true : false; });

			//after step 1 and 2, a new combination is exist
			PrintCombRes<T>(t, vecInt, results);

			//try do step 1 and 2 from front
			i = -1;
		}
	}
}

// t移除ex后的子集res
template<typename T>
void SubVector(const vector<T>& t, const vector<T>& ex, vector<T>& res)
{
	for (uint32_t i = 0; i < t.size(); ++i)
	{
		bool        isExist = false;
		for (uint32_t j       = 0; j < ex.size(); ++j)
		{
			if (t[i] == ex[j])
			{
				isExist = true;
				break;
			}
		}
		if (!isExist)
		{
			res.push_back(t[i]);
		}
	}
}

// 移除子集
template<typename T>
void RemoveSubVec(vector<T>& mulVec, vector<T>& subVec)
{
	for (uint16_t i = 0; i < subVec.size(); ++i)
	{
		for (uint16_t j = 0; j < mulVec.size(); ++j)
		{
			if (subVec[i] == mulVec[j])
			{
				mulVec.erase(mulVec.begin() + j);
				break;
			}
		}
	}
}

// 是否唯一
template<typename T>
bool FindIsOnly(std::vector<T>& v)
{
	for (uint32_t i = 0; i < v.size(); ++i)
	{
		for (uint32_t j = 0; j < v.size(); ++j)
		{
			if (i != j)
			{
				if (v[i] == v[j])
				{
					return false;
				}
			}
		}
	}
	return true;
}
// 移除对象
template<typename T>
bool Remove(std::vector<T>& v, T valu)
{
	auto iter = v.begin();
	while (iter != v.end())
	{
		if (*iter == valu)
		{
			iter = v.erase(iter);
			return true;
		}
		else
			++iter;
	}
	return false;
}










};




﻿#include <iostream>
#include <string>
#include <assert.h>
using namespace std;

#include "green_luacpp.h"

using namespace green_luacpp;

class base_t
{
public:
    base_t():v(789){}
	void dump()
	{
		printf("in %s a:%d\n", __FUNCTION__, v);
	}
	int v;
};
class foo_t: public base_t
{
public:
	foo_t(int b):a(b)
	{
		printf("in %s b:%d this=%p\n", __FUNCTION__, b, this);
	}
	~foo_t()
	{
		printf("in %s\n", __FUNCTION__);
	}
	void print(int64_t a, base_t* p) const
	{
		printf("in foo_t::print a:%ld p:%p\n", (long)a, p);
	}

	static void dumy()
	{
		printf("in %s\n", __FUNCTION__);
	}
	int a;
};

//! lua talbe 可以自动转换为stl 对象
void dumy(map<string, string> ret, vector<int> a, list<string> b, set<int64_t> c)
{
    printf("in %s begin ------------\n", __FUNCTION__);
	for (map<string, string>::iterator it =  ret.begin(); it != ret.end(); ++it)
	{
		printf("map:%s, val:%s:\n", it->first.c_str(), it->second.c_str());
	}
	printf("in %s end ------------\n", __FUNCTION__);
}

class clazz{
public:
    static void static_func(){
        printf("in clazz::%s end ------------\n", __FUNCTION__);
    }
};

static void lua_reg(lua_State* ls)
{
    //! 注册基类函数, LUACTOR() 为构造函数的类型
	green_luacpp_register_t<base_t, LUACTOR()>(ls, "base_t")  //! 注册构造函数
					.def(&base_t::dump, "dump")     //! 注册基类的函数
					.def(&base_t::v, "v");          //! 注册基类的属性

    //! 注册子类，ctor(int) 为构造函数， foo_t为类型名称， base_t为继承的基类名称
	green_luacpp_register_t<foo_t, LUACTOR(int)>(ls, "foo_t", "base_t")
				.def(&foo_t::print, "print")        //! 子类的函数
				.def(&foo_t::a, "a");               //! 子类的字段

	green_luacpp_register_t<>(ls)
				.def(&dumy, "dumy");                //! 注册静态函数

    
    green_luacpp_register_t<clazz, LUACTOR()>(ls, "clazz")
				.def(&clazz::static_func, "static_func"); 
    
}

int main(int argc, char* argv[])
{

	green_luacpp_t green_luacpp;
    try 
    {
        green_luacpp.setModFuncFlag(true);
        //! 注册C++ 对象到lua中
        green_luacpp.reg(lua_reg);
        
        //! 载入lua文件
        green_luacpp.add_package_path("./");
#ifdef _WIN32
        green_luacpp.load_file("../test.lua");
#else
        green_luacpp.load_file("test.lua");
#endif
        //! 获取全局变量
        int var = 0;
        assert(0 == green_luacpp.get_global_variable("test_var", var));
        //! 设置全局变量
        assert(0 == green_luacpp.set_global_variable("test_var", ++var));

        //! 执行lua 语句
        green_luacpp.run_string("print(\"exe run_string!!\")");
        
        //! 调用lua函数, 基本类型作为参数
        int32_t arg1 = 1;
        float   arg2 = 2;
        double  arg3 = 3;
        string  arg4 = "4";
        green_luacpp.call<bool>("test_func", arg1, arg2, arg3,  arg4);
        green_luacpp.call<bool>("Mod:funcTest1", arg1, arg2);
        
        //! 调用lua函数，stl类型作为参数， 自动转换为lua talbe
        vector<int> vec;        vec.push_back(100);
        list<float> lt;         lt.push_back((float)99.99);
        set<string> st;         st.insert("OhNIce");
        map<string, int> mp;    mp["key"] = 200;
        green_luacpp.call<string>("test_stl", vec, lt, st,  mp);
        
        //! 调用lua 函数返回 talbe，自动转换为stl结构
        vec = green_luacpp.call<vector<int> >("test_return_stl_vector");
        lt  = green_luacpp.call<list<float> >("test_return_stl_list");
        st  = green_luacpp.call<set<string> >("test_return_stl_set");
        mp  = green_luacpp.call<map<string, int> >("test_return_stl_map");
        
        //! 调用lua函数，c++ 对象作为参数, foo_t 必须被注册过
        foo_t* foo_ptr = new foo_t(456);
        green_luacpp.call<void>("test_object", foo_ptr);
        
        //! 调用lua函数，c++ 对象作为返回值, foo_t 必须被注册过 
        assert(foo_ptr == green_luacpp.call<foo_t*>("test_ret_object", foo_ptr));
        //! 调用lua函数，c++ 对象作为返回值, 自动转换为基类
        base_t* base_ptr = green_luacpp.call<base_t*>("test_ret_base_object", foo_ptr);
        assert(base_ptr == foo_ptr);
 
    }
    catch (exception& e)
    {
        printf("exception:%s\n", e.what());


    }
#ifdef _WIN32
    system("pause");
#endif
    return 0;
}

#include "Python.h"
#include <iostream>
#include <vector>

void test_python() {
	Py_Initialize();    //初始化

	PyRun_SimpleString("print ('hello')");

    Py_Finalize();      //释放资源
}

bool test_python1(const std::string &psnrlog) {
	Py_Initialize();    //初始化

    #ifndef DEBUG
    std::cout << "为了vpsnr能够正确生成时间维度的分析图，请检查vpsnr和python在同级目录下." << std::endl;
    #endif

	std::string path    = "python";
	std::string cmd_dir = std::string("sys.path.append(\"" + path + "\")");
	
	std::string psnrlog_dir = psnrlog.substr(0, psnrlog.find_last_of('/'));

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");

    PyRun_SimpleString(cmd_dir.c_str());

    // 加载模块
    PyObject* moduleName = PyUnicode_FromString("psnr_graph-Py3");
    
    PyObject* pModule = PyImport_Import(moduleName);
    if (!pModule) {
        std::cout << "Python get module [psnr_graph] failed." << std::endl;
        return false;
    }

    #ifdef DEBUG
    std::cout << "Python get module [psnr_graph] succeed." <<std::endl;
    #endif

    // 加载函数
    PyObject* pv = PyObject_GetAttrString(pModule, "get_psnr_graph");
    if (!pv || !PyCallable_Check(pv)) {
        std::cout << "Can't find funftion [get_psnr_graph]" << std::endl;
        return false;
    }

    #ifdef DEBUG
    std::cout << "Get function [get_psnr_graph] succeed." << std::endl;
    #endif 

    // 设置参数
    PyObject* args = PyTuple_New(2); 
    PyObject* arg1 = Py_BuildValue("s", psnrlog.c_str());    
    PyObject* arg2 = Py_BuildValue("s", psnrlog_dir.c_str());
    PyTuple_SetItem(args, 0, arg1);
    PyTuple_SetItem(args, 1, arg2);

    #ifdef DEBUG    
    std::cout << "第一个参数：" << psnrlog << std::endl;
    std::cout << "第二个参数：" << psnrlog_dir << std::endl;
    #endif

    // 调用函数
    PyObject* pRet = PyObject_CallObject(pv, args);
    if (pRet) {
    	#ifdef DEBUG
        long result = PyLong_AsLong(pRet);
        std::cout << "result:" << result << std::endl;
        #endif

        std::cout << "时间维度可视化执行成功" << std::endl;
    }

    Py_Finalize();      //释放资源

    return true;
}

int main()
{
	// test_python();    
	std::string psnrlog = "psnr/result/psnr.log";

	test_python1(psnrlog);
}
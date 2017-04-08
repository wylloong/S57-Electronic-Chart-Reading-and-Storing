// ENCResolutionV1.0.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "S57Reader.h"
#include <iostream>
#include <string>
#include "stdafx.h"

//创建一个list容器的实例LISTINT   
typedef list<int> LISTINT;

void S57Reader(string encpath)
{
	//用list容器处理整型数据    
	//用LISTINT创建一个名为listOne的list对象   
	LISTINT objllist;
	//声明i为迭代器   
	LISTINT::iterator i;
	/*
	Underwater/awash rock 平水礁     153
	Land area             陆地       71
	Depth contour         等水深线   43
	*/
	//从前面向listOne容器中添加数据   
	//objllist.push_front(43);
	objllist.push_front(71);
	//从后面向listOne容器中添加数据   
	//objllist.push_back(153);
	//过滤指定陆地区域
	double filter_lllat = 52.4;
	double filter_urlat = 53.2;
	double filter_lllon = -170.9;
	double filter_urlon = -169.4;
	OpenS57File(encpath, objllist, filter_lllat, filter_urlat, filter_lllon, filter_urlon);
}

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "******欢迎使用电子海图解析软件******" << endl;
	cout << "  请输入待解析电子海图路径，按Enter键确认" << endl;
	
	//string path;
	//cin >> path;
	//string ENCpath = path;
	//US1BS01M USA

	string ENCpath = "E:\\US1BS01M.000";
	
	S57Reader(ENCpath);
	cout << "  电子海图文件解析完成,按任意键退出！" << endl;
	getchar();
	return 0;
}

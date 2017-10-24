/*******************************************************************
Copyright(c) 2017, waylon
All rights reserved.
Distributed under the BSD license.
*******************************************************************/

#include "S57Reader.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>

typedef list<int> LISTINT;

void S57Reader(string encpath)
{
	LISTINT objllist;

	/*
	Underwater/awash rock 平水礁     153
	Land area             陆地       71
	Depth contour         等水深线   43
	*/

	//choose filter shapelayer
	objllist.push_front(71);   // Land area shapelayer
	//objllist.push_front(43); // Depth contour shapelayer
	//objllist.push_back(153); // Underwater/awash rock shapelayer

	//region
	double filter_lllat = 52.4;
	double filter_urlat = 53.2;
	double filter_lllon = -170.9;
	double filter_urlon = -169.4;

	OpenS57File(encpath, objllist, filter_lllat, filter_urlat, filter_lllon, filter_urlon);
}

int main()
{
	cout << "******欢迎使用电子海图解析软件******" << endl;
	cout << "  请输入待解析电子海图路径，按Enter键确认" << endl;

	//US1BS01M USA
	string ENCpath = "E://US1BS01M.000";

	S57Reader(ENCpath);
	cout << "  电子海图文件解析完成,按任意键退出！" << endl;
	getchar();
	return 0;
}

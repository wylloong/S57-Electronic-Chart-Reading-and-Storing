#include "S57Reader.h"
#include <vector>
#include <fstream>
#include <list>
#include "tinyxml.h"  
#include "tinystr.h" 
#include <iostream>
#include <string>
#include "ogrsf_frmts.h"
#include <time.h>
#include <windows.h>

using namespace std;
typedef list<int> LISTINT;

// function: save geoInfo into xml format
static void SavetoXML(const char* pFilename, EleChartInfo Geolayer)
{
	cout << "  文件保存中..." << endl;
	//创建一个XML的文档对象，对应于XML的整个文档
	TiXmlDocument doc;
	//对应于XML的元素
	TiXmlElement* msg;
	//comment;
	string s;
	//XML中的申明部分
	//TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");

	doc.LinkEndChild(decl);
	//创建一个根元素并连接
	TiXmlElement * root = new TiXmlElement("ENCInfo");
	doc.LinkEndChild(root);
	//注释
	TiXmlComment * comment = new TiXmlComment();
	s = " GeographInfo for " + Geolayer.Chart_Name + " ";
	comment->SetValue(s.c_str());
	root->LinkEndChild(comment);

	// block: GeoInfo
	{
		TiXmlElement *chartNameElement = new TiXmlElement("Chart_Name");
		root->LinkEndChild(chartNameElement);
		TiXmlText *chartNameContent = new TiXmlText(Geolayer.Chart_Name.c_str());
		chartNameElement->LinkEndChild(chartNameContent);
		list<EncLayer> Geogralayerlist = Geolayer.Geolayerlist;
		list<EncLayer>::iterator iter;
		for (iter = Geogralayerlist.begin(); iter != Geogralayerlist.end(); iter++)
		{
			TiXmlElement * layersNode = new TiXmlElement("LayerInfo");
			root->LinkEndChild(layersNode);
			const EncLayer& w = *iter;  //w为一个layer实例
			TiXmlElement * window = new TiXmlElement("Layer");
			layersNode->LinkEndChild(window);
			//设置元素的属性
			window->SetAttribute("id", w.layer_ID);
			//创建元素并连接
			TiXmlElement *layerNameElement = new TiXmlElement("layerName");
			window->LinkEndChild(layerNameElement);
			TiXmlText *layerNameContent = new TiXmlText(w.layer_Name.c_str());
			layerNameElement->LinkEndChild(layerNameContent);
			list<GeoFeature> ww = w.GeoFeaturelist;  //要素的list
			{
				TiXmlElement * FeaturesNode = new TiXmlElement("Features");
				window->LinkEndChild(FeaturesNode);
				list<GeoFeature>::iterator featureiter;
				for (featureiter = ww.begin(); featureiter != ww.end(); featureiter++)
				{
					const GeoFeature& feature = *featureiter;

					TiXmlElement * FeatureEle = new TiXmlElement("Feature");
					FeaturesNode->LinkEndChild(FeatureEle);
					//设置元素的属性
					FeatureEle->SetAttribute("id", feature.Feature_ID);
					//创建元素并连接
					//Feature_Type
					TiXmlElement *TypeElement = new TiXmlElement("Type");
					FeatureEle->LinkEndChild(TypeElement);
					TiXmlText *TypeContent = new TiXmlText(feature.Feature_Type.c_str());
					TypeElement->LinkEndChild(TypeContent);
					//belong_layer
					TiXmlElement *belonglayerElement = new TiXmlElement("layer");
					FeatureEle->LinkEndChild(belonglayerElement);
					TiXmlText *belonglayerContent = new TiXmlText(to_string(feature.Feature_layer).c_str());
					belonglayerElement->LinkEndChild(belonglayerContent);
					//valdco
					TiXmlElement *valdcoElement = new TiXmlElement("valdco");
					FeatureEle->LinkEndChild(valdcoElement);
					TiXmlText *valdcoContent = new TiXmlText(to_string(feature.Feature_valdco).c_str());
					valdcoElement->LinkEndChild(valdcoContent);

					list<EleChartWaypoint> ww = feature.EleChartWaypointlist;  //点的list
					{
						TiXmlElement * PointsNode = new TiXmlElement("wayPoints");
						FeatureEle->LinkEndChild(PointsNode);
						list<EleChartWaypoint>::iterator waypointiter;
						for (waypointiter = ww.begin(); waypointiter != ww.end(); waypointiter++)
						{
							const EleChartWaypoint& point = *waypointiter;

							TiXmlElement * waypointEle = new TiXmlElement("waypoint");
							PointsNode->LinkEndChild(waypointEle);
							//创建元素并连接
							TiXmlElement *IDElement = new TiXmlElement("id");
							waypointEle->LinkEndChild(IDElement);
							TiXmlText *IDContent = new TiXmlText(to_string(point.waypoint_ID).c_str());
							IDElement->LinkEndChild(IDContent);

							TiXmlElement *LngElement = new TiXmlElement("lon");
							waypointEle->LinkEndChild(LngElement);
							TiXmlText *LngContent = new TiXmlText(to_string(point.longitude).c_str());
							LngElement->LinkEndChild(LngContent);

							TiXmlElement *LatElement = new TiXmlElement("lat");
							waypointEle->LinkEndChild(LatElement);
							TiXmlText *LatContent = new TiXmlText(to_string(point.latitude).c_str());
							LatElement->LinkEndChild(LatContent);
						}
					}
				}
			}
			
		}
	}
	doc.SaveFile(pFilename);
}


// read ENC from S57File
void OpenS57File(string lpFileName, list<int> ObjlList, double filter_lllat, double filter_urlat, double filter_lllon, double filter_urlon)
{
	EleChartInfo eleChartInfo;
	list<GeoFeature> GeoFeature_list;
	list<EncLayer> GeoLayer_list;
	EncLayer encLayerInfo;
	OGRRegisterAll();  //注册GDAL/OGR支持的所有格式
	OGRDataSource  *poDS;
	CPLSetConfigOption("OGR_S57_OPTIONS", "SPLIT_MULTIPOINT=ON,ADD_SOUNDG_DEPTH=ON");
	poDS = OGRSFDriverRegistrar::Open(lpFileName.c_str(), FALSE);
	eleChartInfo.Chart_Name = "US1BS01M.000";
	if (poDS == NULL)
	{
		cout << "open FAILED!" << endl;
	}
	cout << "  文件读取中..." << endl;
	const char *pStr = poDS->GetName(); //获取.000文件名称路径
	//根据时间创建xml文件名
	SYSTEMTIME sTime;
	GetSystemTime(&sTime);
	string xmlfileName = to_string(sTime.wYear) + to_string(sTime.wMonth) + to_string(sTime.wDay) + "_" + to_string(sTime.wHour) + to_string(sTime.wMinute) + to_string(sTime.wSecond)+ "_" + "ENCResolution";
	string xmlFilePath = "E:\\" + xmlfileName + ".xml";

	OGRLayer  *poLayer;
	OGREnvelope *pEnvelope = new OGREnvelope;
	//一个OGRDataSource可能包含很多的层，调用 OGRDataSource::GetLayerCount()得到
	poLayer = poDS->GetLayer(0);
	//->结构或类的指针的成员变量或者成员函数，是类或结构体指针访问其成员变量或函数的方式
	poLayer->GetExtent(pEnvelope);
	//得到图层包络区域
	double MaxX = pEnvelope->MaxX;
	double MaxY = pEnvelope->MaxY;
	double MinX = pEnvelope->MinX;
	double MinY = pEnvelope->MinY;

	delete pEnvelope;
	int objectName = 0;  //OBJL编号
	int layerIndex = 0;
	double VALDCO;  //水深
	int layerobjl = 0;
	//按图层分层读取S57中的数据
	for (layerIndex = 0; layerIndex < poDS->GetLayerCount(); layerIndex++)
	{
		poLayer = poDS->GetLayer(layerIndex);   //得到S57单个图层信息
		//读取层里面的features
		OGRFeature *poFeature;		
		poLayer->ResetReading();   //从层的开头开始读取feature
		OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
		encLayerInfo.layer_Name = poFDefn->GetName();
		//调用GetNextFeature遍历所有的feature
		while ((poFeature = poLayer->GetNextFeature()) != NULL)
		{
			int nOBJL=0;
			int pID = poFeature->GetFID();
			int iField;
			objectName = 0;
			//获得一个要素feature的所有字段field
			for (iField = 0; iField < poFDefn->GetFieldCount(); iField++)
			{
				//得到属性数据
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
				//字段名称
				string fieldtemp = poFieldDefn->GetNameRef();
				//字段类型
				OGRFieldType fieldType = poFieldDefn->GetType();
				const char *pFieldTypeName = poFieldDefn->GetFieldTypeName(fieldType);
				string Typetemp = pFieldTypeName;  
				//字段值
				string fieldTypeValue = "";
				if (poFieldDefn->GetType() == OFTInteger)  //属性的类型是整形
					fieldTypeValue = to_string(poFeature->GetFieldAsInteger(iField));
				else if (poFieldDefn->GetType() == OFTReal) //属性的类型是浮点型
					fieldTypeValue = to_string(poFeature->GetFieldAsDouble(iField));
				else if (poFieldDefn->GetType() == OFTString) //属性的类型是字符型
					fieldTypeValue = (poFeature->GetFieldAsString(iField));
				else
					fieldTypeValue = (poFeature->GetFieldAsString(iField));
			}

			int iOBJL = poFDefn->GetFieldIndex("OBJL");   //字段编号
			//获得对应于OBJL的图层编号，如陆地对应objl=71
			layerobjl = poFeature->GetFieldAsInteger(iOBJL);
			encLayerInfo.layer_ID = layerobjl;  //给图层类赋予属性
			//查找list中是否包含iOBJL
			list<int>::iterator index = find(ObjlList.begin(), ObjlList.end(), layerobjl); //查找
			if (index != ObjlList.end())
			{
				if (43 == poFeature->GetFieldAsInteger(iOBJL))
				{
					//Depth contour 等水深线
					VALDCO = poFeature->GetFieldAsDouble("VALDCO");
				}
				else
					VALDCO = -1;
			}
			else
				break;   //不对其它图层进行几何形状解析
			//提取几何数据geometry,并用x、y、z标出来
			OGRGeometry *poGeometry;
			poGeometry = poFeature->GetGeometryRef();  //获取该要素的几何形状,返回一个指向属于OGRFeature的内部几何数据的指针
			if (poGeometry != NULL)
			{
				list<EleChartWaypoint> eleChartWaypoint_list;  //点坐标集合list
				GeoFeature geoFeature;
				geoFeature.Feature_ID = poFeature->GetFID();
				geoFeature.Feature_layer = layerIndex;  //图层编号
				geoFeature.Feature_valdco = VALDCO;
				//只处理陆地信息 71:Land area
				pStr = poGeometry->getGeometryName();
				//InputToTxtFile(TxtFilePath, " 要素几何类型: " +(string)(pStr));
				//int k = poGeometry->getGeometryType();
				if (poGeometry != NULL
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbPoint)
				{
					geoFeature.Feature_Type = "Point";
					//点坐标
					//wkbPlatten() 将一个wkbPoint25D(具有Z)转化为基于2D的类型(wkbPoint)
					OGRPoint *poPoint = (OGRPoint *)poGeometry;
					if (poPoint == NULL)
					{
						//InputToTxtFile(TxtFilePath, "  OGRLinePoint为null！");
					}
					if (poPoint->getY()>filter_lllat &&poPoint->getY()<filter_urlat && poPoint->getX()>filter_lllon && poPoint->getX() < filter_urlon)
					{
						EleChartWaypoint eleChartWaypoint;
						eleChartWaypoint.waypoint_ID = 0;
						eleChartWaypoint.longitude = poPoint->getX();
						eleChartWaypoint.latitude = poPoint->getY();
						eleChartWaypoint_list.push_back(eleChartWaypoint);
					}
				}
				else if (poGeometry != NULL
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbLineString)
				{
					geoFeature.Feature_Type = "Line";
					//线
					OGRLineString *poLine = (OGRLineString *)poGeometry;
					OGRPoint OgrPoint;
					for (int i = 0; i<poLine->getNumPoints(); i++)
					{
						poLine->getPoint(i, &OgrPoint);
						if (OgrPoint.getY()>filter_lllat && OgrPoint.getY()<filter_urlat && OgrPoint.getX()>filter_lllon && OgrPoint.getX() < filter_urlon)
						{
							EleChartWaypoint eleChartWaypoint;
							eleChartWaypoint.waypoint_ID = i;
							eleChartWaypoint.longitude = OgrPoint.getX();
							eleChartWaypoint.latitude = OgrPoint.getY();
							eleChartWaypoint_list.push_back(eleChartWaypoint);
						}
					}
				}
				else if (poGeometry != NULL
					&& wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon)
				{
					geoFeature.Feature_Type = "Polygon";
					//多边形
					OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;
					OGRLineString *poLine = poPolygon->getExteriorRing();  //获取该多边形的外环
					OGRPoint OgrPoint;
					for (int i = 0; i<poLine->getNumPoints(); i++)
					{
						poLine->getPoint(i, &OgrPoint);
						if (OgrPoint.getY()>filter_lllat && OgrPoint.getY()<filter_urlat && OgrPoint.getX()>filter_lllon && OgrPoint.getX() < filter_urlon)
						{
							EleChartWaypoint eleChartWaypoint;
							eleChartWaypoint.waypoint_ID = i;
							eleChartWaypoint.longitude = OgrPoint.getX();
							eleChartWaypoint.latitude= OgrPoint.getY();		
							eleChartWaypoint_list.push_back(eleChartWaypoint);
						}
					}
				}
				else
				{}
				geoFeature.EleChartWaypointlist = eleChartWaypoint_list;
				GeoFeature_list.push_back(geoFeature);  //要素中点的list
			}//geo
			//释放feature拷贝，利用GDAL函数删除，避免win32 heap问题
			OGRFeature::DestroyFeature(poFeature);
			encLayerInfo.GeoFeaturelist=(GeoFeature_list);   //要素的集合
		} //Feature
		eleChartInfo.Geolayerlist.push_back(encLayerInfo);   //图层的集合
	} //图层
	//删除数据文件一个图层从而关闭输入的文件
	OGRDataSource::DestroyDataSource(poDS);
	SavetoXML(xmlFilePath.c_str(), eleChartInfo);  //保存到XML
}
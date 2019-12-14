// 图像定位.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "GxIAPI.h"  
#include "DxImageProc.h"  
#include <opencv2/opencv.hpp>  
#include <iostream>  
#include "quadrangle.h"
#include <math.h>

using namespace std;
using namespace cv;

GX_DEV_HANDLE       m_hDevice;              ///< 设备句柄  
BYTE* m_pBufferRaw;          ///< 原始图像数据  
int64_t             m_nImageHeight;         ///< 原始图像高  
int64_t             m_nImageWidth;          ///< 原始图像宽  
int64_t             m_nPayLoadSize;
int64_t             m_nPixelColorFilter;    ///< Bayer格式  
Mat raw,test,binary;

VideoWriter writer;

//图像回调处理函数  
static void __stdcall OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame)
{
    vector<vector<Point>>contours;
    vector<int> targetContourIndex;
    vector<Point> quadPoints;
    vector<quadrangle> quads;

    if (pFrame->status == 0)
    {
        memcpy(m_pBufferRaw, pFrame->pImgBuf, pFrame->nImgSize);

        memcpy(raw.data, m_pBufferRaw, m_nImageWidth * m_nImageHeight);
        cvtColor(raw, test,COLOR_GRAY2BGR);
        GaussianBlur(raw, raw, Size(3, 3), 1, 1);
        threshold(raw, binary, 70, 255, THRESH_BINARY_INV);
        //adaptiveThreshold(raw, binary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 51, 0);
        //blur(binary, binary, Size(3, 3));
        //threshold(binary, binary, 200, 255, THRESH_BINARY);
        findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        for (int i = 0; i < contours.size(); i++)
        {
            int number = 0;
            if (contours[i].size() < 10 && contours[i].size() > 1000)
                continue;
            RotatedRect Rect = minAreaRect(Mat(contours[i]));
            if (Rect.size.height > Rect.size.width)		//如果宽大于长，重新计算
            {
                Rect.angle -= 90;
                float swap = Rect.size.height;
                Rect.size.height = Rect.size.width;
                Rect.size.width = swap;
            }
            float ratial = Rect.size.width / (float)Rect.size.height;

            if (1 )
            {
                approxPolyDP(contours[i], quadPoints, 40, 1);
                if (quadPoints.size() == 4)
                {
                    quads.push_back(quadrangle(quadPoints));
                    
                }
                
            }
                
        }
        if (quads.size() == 2)
        {
            //let quads[0] be the left one
            if (quads[0].points[0].x > quads[1].points[0].x)
            {
                quadrangle swap = quads[0];
                quads[0] = quads[1];
                quads[1] = swap;
            }
            Point leftY = ( quads[0].points[1] - quads[0].points[0]);
            double distancel = 37. * 100. / sqrt(leftY.x * leftY.x + leftY.y * leftY.y);
            Point rightY = quads[1].points[3] - quads[1].points[2];
            double distancer = 37. * 100. / sqrt(rightY.x * rightY.x + rightY.y * rightY.y);
            double coso = (distancer - distancel) / 7.5;
            double theta = asin(coso) * 180. / 3.1416;

            string ls = to_string(distancel);
            string rs = to_string(distancer);
            string theta_s = to_string(theta);
            string dis = to_string((distancel + distancer) / 2.);
            putText(test, ls, quads[0].points[0] - Point(10, 10), 0, 1, Scalar(50, 210, 50),2);
            putText(test, rs, quads[1].points[2] - Point(10, 10), 0, 1, Scalar(50, 210, 50), 2);
            putText(test, "theta:" + theta_s, Point(100, 100), 0, 1, Scalar(50, 210, 50), 2);
            putText(test, "dis:" + dis, Point(500, 100), 0, 1, Scalar(50, 210, 50), 2);
            for (auto a : quads[0].points)
                circle(test, a, 6, Scalar(120, 120, 250), 2);
            for (auto a : quads[1].points)
                circle(test, a, 6, Scalar(120, 120, 250), 2);
            
        }
        else
        {
            cout << "more than two" << endl;
        }
        writer.write(test);

        imshow("blur", test);
        imshow("binary", binary);
        waitKey(1);

    }

    return;
}



int main(int argc, char* argv[])
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_OPEN_PARAM openParam;
    uint32_t      nDeviceNum = 0;
    openParam.accessMode = GX_ACCESS_EXCLUSIVE;
    openParam.openMode = GX_OPEN_INDEX;
    openParam.pszContent = (char*)"1";
    // 初始化库   
    emStatus = GXInitLib();
    if (emStatus != GX_STATUS_SUCCESS)
    {
        return 0;
    }
    // 枚举设备列表  
    emStatus = GXUpdateDeviceList(&nDeviceNum, 1000);
    if ((emStatus != GX_STATUS_SUCCESS) || (nDeviceNum <= 0))
    {
        return 0;
    }
    //打开设备  
    emStatus = GXOpenDevice(&openParam, &m_hDevice);
    //设置采集模式连续采集  
    emStatus = GXSetEnum(m_hDevice, GX_ENUM_ACQUISITION_MODE, GX_ACQ_MODE_CONTINUOUS);
    emStatus = GXSetInt(m_hDevice, GX_INT_ACQUISITION_SPEED_LEVEL, 2);
    emStatus = GXGetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, new double(500));
    //emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_WHITE_AUTO, GX_BALANCE_WHITE_AUTO_CONTINUOUS);
    bool bColorFliter = false;
    // 获取图像大小  
    emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_nPayLoadSize);
    // 获取宽度  
    emStatus = GXGetInt(m_hDevice, GX_INT_WIDTH, &m_nImageWidth);
    // 获取高度  
    emStatus = GXGetInt(m_hDevice, GX_INT_HEIGHT, &m_nImageHeight);
    test.create(m_nImageHeight, m_nImageWidth, CV_8UC3);
    raw.create(m_nImageHeight, m_nImageWidth, CV_8UC1);
    //判断相机是否支持bayer格式  
    bool m_bColorFilter;
    emStatus = GXIsImplemented(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_bColorFilter);
    if (m_bColorFilter)
    {
        emStatus = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_nPixelColorFilter);
    }
    writer.open("test1.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 15, Size(m_nImageWidth, m_nImageHeight));
    //为存储原始图像数据申请空间  
    m_pBufferRaw = new BYTE[(size_t)m_nPayLoadSize];

    //注册图像处理回调函数  
    emStatus = GXRegisterCaptureCallback(m_hDevice, NULL, OnFrameCallbackFun);
    //发送开采命令  
    emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
    //---------------------  
    //  
    //在这个区间图像会通过OnFrameCallbackFun接口返给用户  
    //Sleep(10000);
    //  
    //---------------------  
    while (waitKey(1) != 'q');
    //发送停采命令  
    emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
    //注销采集回调  
    emStatus = GXUnregisterCaptureCallback(m_hDevice);
    
    if (m_pBufferRaw != NULL)
    {
        delete[]m_pBufferRaw;
        m_pBufferRaw = NULL;
    }
    emStatus = GXCloseDevice(m_hDevice);
    emStatus = GXCloseLib();
    return 0;
}
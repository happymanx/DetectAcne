#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;

IplImage *src;
IplImage *gray;
IplImage *hsv;
IplImage *normalizedGray;
IplImage subtraction;
IplImage *binary;
Mat subtractImage;

int lowThreshold;
int const max_lowThreshold = 255;

const char *window_source = "Source Image";
const char *window_gray = "Gray Image";
const char *window_normalizedGray = "normalized Gray Image";
const char *window_subtraction = "Subtraction Image";
const char *window_binary = "Binary Image";
const char *window_hsv = "HSV Image";
const char *window_hsv_to_v = "HSV to V Image";
const char *window_count_label = "Count and Label Image";
const char *imageName = "pox.jpg";

Scalar whiteColor(255,255,255);
Scalar redColor(0,0,255);
Scalar greenColor(0,255,0);

// 擷取 pixel 資訊
#define IMGDATA(image,i,j,k) *((uchar *)&image->imageData[(i)*(image->widthStep) + (j)*(image->nChannels) + (k)])

void normalizeGray()
{
    int largestPixelValue = 0;
    
    // 找出灰階圖最大值
    for(int i=0;i<gray->height;i++){
        for(int j=0;j<gray->width;j++){
            uchar intensity = CV_IMAGE_ELEM(gray, uchar, i, j);
            if (largestPixelValue < intensity) {
                largestPixelValue = intensity;
            }
        }
    }
    cout << "largestPixelValue: " << largestPixelValue << endl;
    
    for(int i=0;i<gray->height;i++){
        for(int j=0;j<gray->width;j++){
            uchar intensity = CV_IMAGE_ELEM(gray, uchar, i, j);
            int newIntensity = ((float)intensity / largestPixelValue) * 255;
//            cout << "newIntensity: " << newIntensity << endl;
            IMGDATA(normalizedGray,i,j,0) = (uchar)newIntensity;
        }
    }

    cvShowImage(window_normalizedGray, normalizedGray);
}

void BinaryTest(int, void*)
{
    // 轉換資料結構 Mat -> IplImage
    subtraction = IplImage(subtractImage);
    
    cvThreshold(&subtraction, binary, lowThreshold, 255, THRESH_BINARY);
    cvShowImage(window_binary, binary);
    
    cout << "Threshold: " << lowThreshold << endl;
    
    vector<vector<Point>> contours; // Vector for storing contour
    Mat binaryImage(binary, 0);
    findContours(binaryImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    Mat countAndLabelImage(src, 1);
    Rect bounding_rect;

    double area;// 面積
    int matchNumber = 0;// 符合
    int unmatchNumber = 0;// 不符合
    // 畫出每一個輪廓
    for(int i = 0; i < contours.size(); i++) // Iterate through each contour
    {
        area = contourArea(contours[i], false);
        drawContours(countAndLabelImage, contours, i, whiteColor, CV_FILLED);
        bounding_rect = boundingRect(contours[i]);
        // 畫出輪廓的邊框
        if (area < 20 || area > 7000) {// 超過或小於就略過
            rectangle(countAndLabelImage, bounding_rect, greenColor, 1, 8, 0);
            unmatchNumber++;
        }
        else {
            rectangle(countAndLabelImage, bounding_rect, redColor, 1, 8, 0);
            matchNumber++;
        }
    }
    cout << "Match Number: " << matchNumber << endl;
    cout << "Unmatch Number: " << unmatchNumber << endl;

    imshow(window_count_label, countAndLabelImage);
}

int main(){
    // 讀取影像
    src = cvLoadImage(imageName);

    // 創造和src一樣IplImage資料結構
    hsv = cvCreateImage(cvGetSize(src), 8, 3);
    gray = cvCreateImage(cvGetSize(src), 8, 1);
    normalizedGray = cvCreateImage(cvGetSize(src), 8, 1);
    binary = cvCreateImage(cvGetSize(src), 8, 1);
    
    // 影像轉換到 HSV & Gray 顏色空間
    cvCvtColor(src, hsv, CV_BGR2HSV);
    cvCvtColor(src, gray, CV_BGR2GRAY);
    
    normalizeGray();
    
    // 將HSV拆成三個頻道
    Mat hsv_channels[3];
    split(hsv, hsv_channels);
    // 顯示HSV的V
    imshow(window_hsv_to_v, hsv_channels[2]);

    // 轉換資料結構 IplImage -> Mat
    Mat normalizedGrayImage(normalizedGray, 0);
    // HSV的V頻道圖減掉正規化灰階圖
    subtract(hsv_channels[2], normalizedGrayImage, subtractImage);
    imshow(window_subtraction, subtractImage);
    
    cvNamedWindow(window_binary, CV_WINDOW_AUTOSIZE);
    createTrackbar("Threshold: ", window_binary, &lowThreshold, max_lowThreshold, BinaryTest);
    
    // 預設閾值
    lowThreshold = 40;
    BinaryTest(lowThreshold, 0);

    cvShowImage(window_source, src);
    cvShowImage(window_gray, gray);
    cvShowImage(window_hsv, hsv);
    
    cvWaitKey(0);
    return EXIT_SUCCESS;
}

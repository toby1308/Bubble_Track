// Track_Detector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <sstream>

using namespace std;
using namespace cv;
const char* image_window = "Source Image";
//const char* result_window = "Result window";
int font_size = 1;
Scalar font_color(0, 0, 0);
int font_weight = 2;

void save(Mat image,string path) {
    bool isSuccess = imwrite(path, image);
    if (isSuccess == false) {
        cout << "Failed to save image" << endl;
        cin.get();
    }
    else {
        cout << "Image saved" << endl;
    }
}

int** TemplateMatching(Mat source, Mat templ, int objectNum) {
    int** arr = new int* [objectNum];
    Mat result;
    double minVal;
    double maxVal;
    Point minLoc;
    Point matchLoc;
    Point maxLoc;
    int j = 0;
    for (int k = 0; k < objectNum; k++, j++) {
        matchTemplate(source, templ, result, TM_SQDIFF_NORMED);
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
        matchLoc = minLoc;
        arr[k] = new int[objectNum];
        arr[k][0] = matchLoc.x;
        arr[k][1] = matchLoc.y;
        for (int i = 0; i < templ.cols; i++) {
            for (int j = 0; j < templ.rows; j++) {
                source.at<Vec2b>(matchLoc.y + j, (matchLoc.x + i) / 2) = 0;
            }
        }
    }
    return arr;
}

int Table(int binary) {
    switch (binary) {
    case(21100001):
        return 1;
        break;
    case(21010010):
        return 2;
        break;
    case(10110011):
        return 3;
        break;
    case(20110100):
        return 4;
        break;
    case(11010101):
        return 5;
        break;
    case(11100110):
        return 6;
        break;
    case(20000111):
        return 7;
        break;
    case(11111000):
        return 8;
        break;
    case(20011001):
        return 9;
        break;
    case(20101010):
        return 0;
        break;
    default:
        return 0;
        break;
    }
}

void Barcode(Mat source, int x, int y, Mat templ) {
    int barcode[80];
    memset(barcode, 0, sizeof(barcode));
    int textNum[10];
    memset(textNum, 0, sizeof(textNum));
    int adjust;
    std::ostringstream oss;
    for (int N = 0; N < 2; N++) {
        int test = 0;
        int index = 0;
        int count = 0;
        int width = 29;
        int norm = 12;
        int threshold = 100;
        while (index != 100) {
            test = int(source.at<Vec2b>(y + index, x / 2 + 66 - N * 44)[0]);
            if (test < threshold - 10 && test > 50) {
                count += 1;
            } //makes sure its a barcode value and not random black dot
            if (count > 2 && test > threshold - 20) {  //found barcode
                for (int i = 0; i < 40; i++) {//barcode sections
                    for (int n = 0; n < 29; n++) {//each section value
                        barcode[i + N * 40] += source.at<Vec2b>(y + index - count / 2 - norm + i * 29 + n, x / 2 + 66 - N * 44)[0];
                    }//total value of each section
                    if (barcode[i + N * 40] / width > threshold) {
                        barcode[i + N * 40] = 0;
                    }
                    else {
                        barcode[i + N * 40] = 1;
                    }
                }
                index = 100;
            }
            else {
                index += 1;
            }
        }
    }
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 8; j++) {
            if (j == 0) {
                adjust = 1;
            }
            else {
                adjust = 0;
            }
            textNum[i] += (barcode[i * 8 + j] + adjust) * pow(10, 7 - j);
        }
        textNum[i] = Table(textNum[i]);
    }
    oss << "Photo: " << textNum[1] << textNum[2] << textNum[3] << textNum[4] << "; Roll: " << textNum[6] << textNum[7] << textNum[8] << textNum[9] << "; Cam: " << textNum[5] << "; Expan: " << textNum[0];
    std::string var = oss.str();
    Point text_position(source.cols / 2, y / 2);
    putText(source, var, text_position, FONT_HERSHEY_COMPLEX, font_size, font_color, font_weight);
}

void find_track_manual(Mat source,int x, int y) {
    int j = 0;
    int count = 0;
    int test;
    //rectangle(source, Point(500, 500), Point(600, 600), Scalar(0, 0, 255));
    while (j == 0) {
        test = int(source.at<Vec2b>(y + count, x)[0]);
        cout << count << endl;
        if (test < 90 && test > 50) {
            rectangle(source,Point(y+count,x),Point(y + count + 100,x + 100),Scalar(0,255,255));
            j = 1;
        }
        count += 1;
        if (count > 1000) {
            rectangle(source, Point(y + count, x), Point(y + count + 100, x + 100), Scalar(0, 0, 255));
            j = 1;
        }
    }
    imshow(image_window, source);
}


void find_track_temp(Mat source, int x_value, int upper_y_val,int lower_y_val){
    //itergrate through matches until it fails or the quality isn't high enough and reutrn the locatin of each one in an array, can then follow up by decoding the 
    // the track into an equation and then finally would be able to follow through with the track equation until it breaks.
    int** location_array;
    Mat image_section = source(Range(upper_y_val,lower_y_val),Range(x_val,x_val+100));
    location_array = TemplateMatching(image_section,track_template,20);//need to add in the template for the track
    //create rectangles around the tracked locations to see best fits
    //need to create a seperate test to see if all the given locations actually contain a track
    //removed all non-matching tracks
    //due to the large amount of possible tracks threshold matching is probably better(would need to add numpy)
}

// need to find a way to detect paths accurately, best idea so far is to maybe take a staring section and either go through each coloumn making down spots
// then plot the data to find the most likely paths 
// or possibly use template matching to find the paths from that section
int main()
{
    int** arr1;
    int** arr2;
    string img_path = "input_images/img_001.tif";
    string templ_path = "referance_img/Fadutial_Mark.png";
    string barcode_path = "referance_img/Barcode2.png";
    string save_path = "output_images/MyImage.png";

    //cout << "Please enter photo directory: " << endl;
    //cin >> img_path;
    //cout << "Please enter where you'd like to save the photos to: " << endl;
    //cin >> save_path;


    Mat image = imread(img_path, IMREAD_GRAYSCALE);
    resize(image, image, { 4788,1600 });
    Mat templ = imread(templ_path, IMREAD_GRAYSCALE);
    resize(templ, templ, { 140,126 });
    Mat barTempl = imread(barcode_path, IMREAD_GRAYSCALE);
    resize(barTempl, barTempl, { barTempl.cols * 2,barTempl.rows * 2 });
    Mat img_display;
    image.copyTo(img_display);

    arr1 = TemplateMatching(image, templ, 2);//Finds fedutial mark locations
    img_display = img_display(Range(arr1[1][1], img_display.rows), Range(0, img_display.cols));
    arr2 = TemplateMatching(image, barTempl, 1);//Find barcode location
    Barcode(img_display, arr2[0][0], arr2[0][1] - arr1[1][1], barTempl);
    //find_track(img_display, 1000, 500);
    find_track(img_display, arr1[1][0],arr1[1][1],arr1[0][1]);
    //rectangle(img_display, Point(500, 500), Point(600, 600), Scalar(0, 255, 255));

    img_display = img_display(Range(0, arr1[0][1]), Range(arr1[0][0], img_display.cols - 100));
    save(img_display,save_path);
    resize(img_display, img_display, { img_display.cols / 2,img_display.rows / 2 });
    //imshow(image_window, img_display);
    waitKey(0);
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

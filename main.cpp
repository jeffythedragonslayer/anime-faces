#include <cv.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <highgui.h>
using namespace cv;
using namespace std; 
typedef unsigned int uint;

Mat src, src_gray, drawing;
int thresh = 100;
RNG rng(12345);

Vec2i blue_seed_point, red_seed_point;

Scalar WHITE(255, 255, 255);
Scalar RED  (255, 0,   0);
Scalar GREEN(0,   255, 0);
Scalar BLUE (0,   0,   255);

float square(float x) {return x*x;}

string character = "Oak.jpg";

struct FeatureVector
{
        float ratio;
        Vec3f hair_color, face_color;

        float distance_to(FeatureVector const&);
};

float distance(FeatureVector const& a, FeatureVector const& b)
{
        float dist = 0;
        dist += 3*square(a.ratio - b.ratio);

        dist += square(a.face_color[0] - b.face_color[0]);
        dist += square(a.face_color[1] - b.face_color[1]);
        dist += square(a.face_color[2] - b.face_color[2]);

        dist += square(a.hair_color[0] - b.hair_color[0]);
        dist += square(a.hair_color[1] - b.hair_color[1]);
        dist += square(a.hair_color[2] - b.hair_color[2]); 
        return dist;
}

vector<FeatureVector> fvecs; 
FeatureVector fvec;

void count()
{
        uint count_hair = 0, count_face = 0; 
        Vec3f total_hair(0, 0, 0), total_face(0, 0, 0);

        for( uint y = 0; y < drawing.rows; ++y ){
                for( uint x = 0; x < drawing.cols; ++x ){
                        if( drawing.at<Vec3b>(y,x) == Vec3b(255, 0, 0) ){
                                total_hair += src.at<Vec3b>(y, x);
                                ++count_hair;
                        } else if( drawing.at<Vec3b>(y,x) == Vec3b(0, 0, 255) ){
                                total_face += src.at<Vec3b>(y, x);
                                ++count_face;
                        } 
                }
        }

        total_hair[0] /= count_hair * 255;
        total_hair[1] /= count_hair * 255;
        total_hair[2] /= count_hair * 255;
        total_face[0] /= count_face * 255;
        total_face[1] /= count_face * 255;
        total_face[2] /= count_face * 255;

        float ratio = count_hair * 1.f / count_face;

        system("cls");
        cout << "Click the features.  Feature vector:\n";
        cout << "Red pixels: "  << count_hair << '\n';
        cout << "Blue pixels: " << count_face << '\n';
        cout << "Ratio: "       << ratio << '\n';
        cout << "Hair color: "  << total_hair(0) << ' ' << total_hair(1) << ' ' << total_hair(2) << '\n';
        cout << "Face color: "  << total_face(0) << ' ' << total_face(1) << ' ' << total_face(2) << '\n';

        fvec.face_color = total_face;
        fvec.hair_color = total_hair;
        fvec.ratio      = ratio;
}

void thresh_callback(int, void*)
{
        Mat                   threshold_out;
        vector<vector<Point>> contours;
        vector<Vec4i>         hierarchy;

        threshold(src_gray, threshold_out, thresh, 255, THRESH_BINARY);
        imshow("threshold", threshold_out);
        findContours(threshold_out, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

        vector<vector<Point>> contours_poly(contours.size()); 
        for( int i = 0; i < contours.size(); ++i ) approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);

        drawing = Mat::zeros(threshold_out.size(), CV_8UC3);
        for( int i = 0; i < contours.size(); ++i ) drawContours(drawing, contours_poly, i, WHITE, 1, 8, vector<Vec4i>(), 0, Point());

        namedWindow("Contours", CV_WINDOW_AUTOSIZE);
        imshow("Contours", drawing);
}

void load_feature_vecs()
{
        ifstream f("C:/Users/Game/Desktop/Anime/feature_vectors.txt");
        while( !f.eof() ){
                FeatureVector fv;
                string trash;
                f >> trash >> fv.ratio;
                f >> trash >> trash >> fv.hair_color(0) >> fv.hair_color(1) >> fv.hair_color(2);
                f >> trash >> trash >> fv.face_color(0) >> fv.face_color(1) >> fv.face_color(2);
                fvecs.push_back(fv);
                
        }
}

void mouse_event(int event, int x, int y, int flags, void* userdata)
{
        switch( event ){
                case EVENT_LBUTTONDOWN:
                        cout << "working...\n";
                        blue_seed_point = Vec2i(x, y); 
                        floodFill(drawing, blue_seed_point, RED);
                        imshow("Contours", drawing);
                        cout << "ok\n";
                        break;
                case EVENT_RBUTTONDOWN:
                        cout << "working...\n";
                        red_seed_point = Vec2i(x, y); 
                        floodFill(drawing, red_seed_point, BLUE);
                        imshow("Contours", drawing);
                        cout << "ok\n";
                        break;
                case EVENT_MBUTTONDOWN:{
                        ofstream f("C:/Users/Game/Desktop/Anime/feature_vectors.txt", ios::app | ios::out);
                        f << "Character: "   << character << '\n';
                        f << "Ratio: "       << to_string(fvec.ratio) << '\n';
                        f << "Hair color: "  << to_string(fvec.hair_color(0)) << ' ' << fvec.hair_color(1) << ' ' << fvec.hair_color(2) << '\n';
                        f << "Face color: "  << to_string(fvec.face_color(0)) << ' ' << fvec.face_color(1) << ' ' << fvec.face_color(2) << '\n';
                        f << '\n';
                        break;}
                case EVENT_MOUSEMOVE: 
                        break;
        }
        count();
}

int main(int argc, char** argv)
{
        //src = imread("C:/Users/Game/Desktop/Anime/" + character, 1);
        src = imread("C:/test.png", 1);

        cvtColor(src, src_gray, CV_BGR2GRAY);
        imshow("gray", src_gray);

        char* source_window = "Source";
        namedWindow(source_window, CV_WINDOW_AUTOSIZE);
        setMouseCallback("Source", mouse_event, nullptr);
        imshow(source_window, src);

        createTrackbar("Threshold:", "Source", &thresh, 255, thresh_callback);
        thresh_callback(0, 0);

        waitKey(0);
        return(0);
}
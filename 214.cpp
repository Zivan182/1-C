#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include<iostream>
using namespace cv;
using namespace std;

using std::pair;
using std::vector;



// длина отрезка
float len(const Vec4i& v) {
    int x = v[2] - v[0];
    int y = v[3] - v[1];
    return sqrt(float(x * x + y * y));
}

// минимум из двух точек
pair<int, int> minimum(int x1, int y1, int x2, int y2) {
    if ((x1 < x2) || (x1 == x2 && y1 < y2)) {
        return { x1, y1 };
    }
    else return { x2, y2 };
}

// максимум из двух точек
pair<int, int> maximum(int x1, int y1, int x2, int y2) {
    if ((x1 > x2) || (x1 == x2 && y1 > y2)) {
        return { x1, y1 };
    }
    else return { x2, y2 };
}

// синус угла между отрезками
float sinus(const Vec4i& i, const Vec4i& j) {
    int xi = i[2] - i[0];
    int yi = i[3] - i[1];
    int xj = j[2] - j[0];
    int yj = j[3] - j[1];
    return (float(xi * yj - xj * yi) / (len(i) * len(j)));
}


// находим отрезки по изображению
vector<Vec4i> find_segments(const char* filename) {
    Mat dst, cdstP;
    Mat src = imread(samples::findFile(filename), IMREAD_GRAYSCALE);

    threshold(src, dst, 100, 255, 1);

    cvtColor(dst, cdstP, COLOR_GRAY2BGR);

    // Probabilistic Line Transform
    vector<Vec4i> linesP; 
    HoughLinesP(dst, linesP, 1, CV_PI / 180, 150, 50, 10); 
 
    // один отрезок может встречаться несколько раз из за неточности, поэтому удалим повторения
    vector<bool> flag(linesP.size(), true);
    for (int i = 0; i < linesP.size(); i++) {
        for (int j = i + 1; j < linesP.size(); j++) {
            Vec4i li = linesP[i];
            Vec4i lj = linesP[j];
            int xi = li[2] - li[0];
            int yi = li[3] - li[1];
            int xj = lj[2] - lj[0];
            int yj = lj[3] - lj[1];
            float leni = sqrt(xi * xi + yi * yi);
            float lenj = sqrt(xj * xj + yj * yj);
            if (abs(sinus(li, lj)) < 0.01) {
                auto l = minimum(li[0], li[1], lj[0], lj[1]);
                auto r = maximum(li[2], li[3], lj[2], lj[3]);
                flag[i] = false;
                linesP[j] = {l.first, l.second, r.first, r.second};
                break;
            }
        }
    }

    vector<Vec4i> ans;
    for (int i = 0; i != linesP.size(); ++i) {
        if (flag[i]) {
            ans.push_back(linesP[i]);
        }
    }
    return ans;

}

double f(double x, double y, double a, double b, double c) {
    return (a * x + b * y + c);
}

// проверка пересечения
bool intersect(const Vec4i& s1, const Vec4i& s2) {
    double a1, b1, c1, a2, b2, c2;
    a1 = s1[1] - s1[3];
    b1 = s1[2] - s1[0];
    c1 = s1[0] * s1[3] - s1[2] * s1[1];
    a2 = s2[1] - s2[3];
    b2 = s2[2] - s2[0];
    c2 = s2[0] * s2[3] - s2[2] * s2[1];

    if ((abs(s1[0] - s2[0]) < 5) && (abs(s1[1] - s2[1]) < 5))
        return false;
    if ((abs(s1[2] - s2[2]) < 5) && (abs(s1[3] - s2[3]) < 5))
        return false;

    bool flag1 = (f(s1[0], s1[1], a2, b2, c2) * f(s1[2], s1[3], a2, b2, c2) <= 0);
    bool flag2 = (f(s2[0], s2[1], a1, b1, c1) * f(s2[2], s2[3], a1, b1, c1) <= 0);
    if (!(flag1 && flag2)) {
        return false;
    }
    return true;
}


// возвращает количество пересечений в множестве отрезков
int count_of_intersect(const vector<Vec4i>& segments) {
    int count = 0;
    for (int i = 0; i != segments.size(); ++i) {
        for (int j = i + 1; j != segments.size(); ++j) {
            if (intersect(segments[i], segments[j])) {
                ++count;
            }
        }
    }
    return count;
}


// возвращает количество пересечений в множестве отрезков, изображенных на картинке
int count_of_intersect_from_image(const char* filename) {
    return count_of_intersect(find_segments(filename));
}

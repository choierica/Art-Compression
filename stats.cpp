#include "stats.h"

stats::stats(PNG & im) {
    // iterate each of img
    // get hueX, hueY, sumSat, sumLum
    unsigned int height = im.height();
    unsigned int width = im.width();
    //pair<int, int> lr = make_pair(height - 1, width - 1);
    double tempHX = 0;
    double tempHY = 0;
    double tempS = 0;
    double tempL = 0;
    HSLAPixel* currPixel;

    hist.resize(width, vector<vector<int>> (height, vector<int>(36)));

    for (unsigned int y = 0; y < height; y++){ // previously i
        vector<double> rows;
        sumHueX.push_back(rows);
        sumHueY.push_back(rows);
        sumSat.push_back(rows);
        sumLum.push_back(rows);

        for (unsigned int x = 0; x < width; x++) { // previously j
            currPixel = im.getPixel(x, y);

            if (y == 0 && x == 0) { // base case
                tempHX = getHueX(currPixel);
                tempHY = getHueY(currPixel);
                tempS = currPixel->s;
                tempL = currPixel->l;
            } else if (y == 0) { // height == 0 -> first row
                for (int bucket = 0; bucket < 36; bucket++) {
                    hist[x][0][bucket] = hist[x - 1][0][bucket]; // add previous value
                }
                tempHX = sumHueX[y][x - 1] + getHueX(currPixel);
                tempHY = sumHueY[y][x - 1] + getHueY(currPixel);
                tempS = sumSat[y][x - 1] + currPixel->s;
                tempL = sumLum[y][x - 1] + currPixel->l;
            } else if (x == 0) { // width == 0 -> first column
                for (int bucket = 0; bucket < 36; bucket++) {
                    hist[0][y][bucket] = hist[0][y - 1][bucket]; // add previous value
                }
                tempHX = sumHueX[y - 1][x] + getHueX(currPixel);
                tempHY = sumHueY[y - 1][x] + getHueY(currPixel);
                tempS = sumSat[y - 1][x] + currPixel->s;
                tempL = sumLum[y - 1][x] + currPixel->l;
            } else { // sum = left + up - leftup + curr
                for (int bucket = 0; bucket < 36; bucket++) {
                    hist[x][y][bucket] = hist[x][y-1][bucket] + hist[x-1][y][bucket] - hist[x-1][y-1][bucket];
                }
                tempHX = sumHueX[y][x - 1] + sumHueX[y - 1][x] - sumHueX[y - 1][x - 1] + getHueX(currPixel);
                tempHY = sumHueY[y][x - 1] + sumHueY[y - 1][x] - sumHueY[y - 1][x - 1] + getHueY(currPixel);
                tempS = sumSat[y][x - 1] + sumSat[y - 1][x] - sumSat[y- 1][x - 1] + currPixel->s;
                tempL = sumLum[y][x - 1] + sumLum[y - 1][x] - sumLum[y - 1][x - 1] + currPixel->l;
            }
            int i = (int) currPixel->h / 10;
            hist[x][y][i]++;

            sumHueX[y].push_back(tempHX);
            sumHueY[y].push_back(tempHY);
            sumSat[y].push_back(tempS);
            sumLum[y].push_back(tempL);
        }
    }
}

// given any rectangle, build a histogram of color frequencies
// over the rectangle using the hist private member variable
// described above.
vector<int> stats::buildHist(pair<int,int> ul, pair<int,int> lr){
    vector<int> totalVec = hist[lr.first][lr.second];
    vector<int> upVec(36, 0);
    vector<int> leftVec(36, 0);
    vector<int> ulVec(36, 0);

    if (ul.second != 0) { // if y != 0, assign upCorner
        upVec = hist[lr.first][ul.second - 1];
    }
    if (ul.first != 0) { // if x != 0 and y == 0, assign leftCorner
        leftVec = hist[ul.first - 1][lr.second];
    }
    if (ul.first != 0 && ul.second != 0) { // if x & y != 0, Total - upCorner - leftCorner + ulcorner
        ulVec = hist[ul.first -1][ul.second -1];
    }
    for(int i = 0; i < 36; i++){
       totalVec[i] = totalVec[i] - leftVec[i] - upVec[i] + ulVec[i];
    }

    return totalVec;
}

double stats::getHueX(HSLAPixel* pixel) {
    double s = pixel->s;
    double h = pixel->h;
    double hueX = s * (cos(h * PI / 180.0));

    return hueX;
}

double stats::getHueY(HSLAPixel* pixel) {
    double s = pixel->s;
    double h = pixel->h;
    double hueY = s * (sin(h * PI / 180.0));

    return hueY;
}
// given a rectangle, return the number of pixels in the rectangle
/* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::rectArea(pair<int,int> ul, pair<int,int> lr) {
    int width = (lr.first - ul.first) + 1;
    int height = (lr.second - ul.second) + 1;
    return width * height;
}

// given a rectangle, return the average color value over the rect.
/* Each color component of the pixel is the average value of that
* component over the rectangle.
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
// The average hue value can be computed from the average X and
// Y values using the arctan function. You should research the
// details of this. Finally, please set the average alpha channel to
// 1.0.
HSLAPixel stats::getAvg(pair<int,int> ul, pair<int,int> lr){
    long area = rectArea(ul, lr);
    pair<int, int> leftCorner = make_pair(0, 0);
    pair<int, int> upCorner = make_pair(0, 0);
    pair<int, int> ulCorner = make_pair(0, 0);

    double totalHueX = sumHueX[lr.second][lr.first];
    double totalHueY = sumHueY[lr.second][lr.first];
    double totalSat = sumSat[lr.second][lr.first];
    double totalLum = sumLum[lr.second][lr.first];

    if (ul.second != 0) { // if y != 0, assign upCorner
        upCorner.first = lr.first;
        upCorner.second = ul.second - 1;
    }
    if (ul.first != 0) { // if x != 0 and y == 0, assign leftCorner
        leftCorner.first = ul.first - 1;
        leftCorner.second = lr.second;
    }
    if (ul.first != 0 && ul.second != 0) { // if x & y != 0, Total - upCorner - leftCorner + ulcorner
        ulCorner.first = ul.first - 1;
        ulCorner.second = ul.second - 1;
    }

    totalHueX = totalHueX - getSumHueX(leftCorner) - getSumHueX(upCorner) + getSumHueX(ulCorner);
    totalHueY = totalHueY - getSumHueY(leftCorner) - getSumHueY(upCorner) + getSumHueY(ulCorner);
    totalSat = totalSat - getSumSat(leftCorner) - getSumSat(upCorner) + getSumSat(ulCorner);
    totalLum = totalLum - getSumLum(leftCorner) - getSumLum(upCorner) + getSumLum(ulCorner);

    HSLAPixel avg;

    double avgHue = getHueAvg(totalHueX, totalHueY);
    double avgSat = totalSat / area;
    double avgLum = totalLum / area;

    avg.h = avgHue;
    avg.s = avgSat;
    avg.l = avgLum;
    avg.a = 1.0;
    return avg;
}

double stats::getHueAvg(double hueX, double hueY) {
    double hTemp = atan2( (hueY),  (hueX));
    double hDeg = hTemp * 180.0 / PI;
    if (hDeg < 0) {
        hDeg += 360.0;
    }
    hDeg = fmod(hDeg, 360.0);
    return hDeg;
}

double stats::getSumHueX(pair<int, int> xy) {
    if (xy.first == 0 && xy.second == 0) {
        return 0;
    }
    return sumHueX[xy.second][xy.first];
}

double stats::getSumHueY(pair<int, int> xy) {
    if (xy.first == 0 && xy.second == 0) {
        return 0;
    }
    return sumHueY[xy.second][xy.first];
}

double stats::getSumSat(pair<int, int> xy) {
    if (xy.first == 0 && xy.second == 0) {
        return 0;
    }
    return sumSat[xy.second][xy.first];
}

double stats::getSumLum(pair<int, int> xy) {
    if (xy.first == 0 && xy.second == 0) {
        return 0;
    }
    return sumLum[xy.second][xy.first];
}

// given a distribution over colors, and the area of the region,
// return the total entropy.
/* entropy is computed from the distn parameter, as
 * follows: E = -Sum(p_i log(p_i)), where p_i is the fraction of
 * pixels in bin i, and the sum is taken over all the bins.
 * bins holding no pixels should not be included in the sum. */

// takes a distribution and returns entropy
// partially implemented so as to avoid rounding issues.
// !! SHOULD NEVER BE NEGATIVE !!
double stats::entropy(vector<int> & distn,int area){
    double entropy = 0.;
    for (int i = 0; i < 36; i++) {
        if (distn[i] > 0 ) 
            entropy += ((double) distn[i]/(double) area) 
                                    * log2((double) distn[i]/(double) area);
    }
    return  -1 * entropy;
}

// given any rectangle, return the total entropy over that rect.
/* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
/* entropy is computed from the (private member) distn table, as
 * follows: E = -Sum(p_i log(p_i)), where p_i is the fraction of
 * pixels in bin i, and the sum is taken over all the bins.
 * bins holding no pixels should not be included in the sum. */
double stats::entropy(pair<int,int> ul, pair<int,int> lr){
    vector<int> histogram = buildHist(ul, lr);
    int area = rectArea(ul, lr);
    return entropy(histogram, area);
}


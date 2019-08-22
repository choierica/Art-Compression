#define CATCH_CONFIG_MAIN
#include "cs221util/catch.hpp"
#include <vector>
#include <sys/stat.h>
#include <iostream>
#include "cs221util/PNG.h"
#include "cs221util/HSLAPixel.h"
#include "stats.h"
#include "toqutree.h"

using namespace std;
using namespace cs221util;


TEST_CASE("stats::basic rectArea","[weight=1][part=stats]"){

    PNG data; data.resize(2,2);

    stats s(data);
    pair<int,int> ul(0,0);
    pair<int,int> lr(1,1);
    long result = s.rectArea(ul,lr);

    REQUIRE(result == 4);
    
}

TEST_CASE("stats::basic getAvg","[weight=1][part=stats]"){
    PNG data; data.resize(2,1);
    for (int i = 0; i < 2; i ++){
        for (int j = 0; j < 1; j++){
            HSLAPixel * p = data.getPixel(i,j);
            //p->h = 135 * j + i * 90;
            p->h = 165;
            p->s = 0.5;
            p->l = 0.4;
            p->a = 1.0;
        }
    }
    stats s(data);
    pair<int,int> ul(1,0);
    pair<int,int> lr(1,0);
    HSLAPixel result = s.getAvg(ul,lr);
    s.buildHist(ul, lr);

    HSLAPixel expected(230, 0.5, 0.4);

    REQUIRE(result == expected);
}

TEST_CASE("stats::basic entropy","[weight=1][part=stats]"){
    PNG data; data.resize(2,2);
    for (int i = 0; i < 2; i ++){
        for (int j = 0; j < 2; j++){
            HSLAPixel * p = data.getPixel(i,j);
            p->h = 135*j + i * 90;
            p->s = 1.0;
            p->l = 0.5;
            p->a = 1.0;
        }
    }
    stats s(data);
    pair<int,int> ul(0,0);
    pair<int,int> lr(1,1);
    long result = s.entropy(ul,lr);

    REQUIRE(result == 2);
}

TEST_CASE("toqutree::basic ctor render","[weight=1][part=toqutree]"){
    PNG img;
    img.readFromFile("images/minecraft.png");

    toqutree t1(img,7);
    PNG out = t1.render();
    //out.convert();
    out.writeToFile("images/minecraft copy.png");
    cout << t1.size() << endl;

    REQUIRE(out==img);
}

TEST_CASE("toqutree::basic copy","[weight=1][part=toqutree]"){
    PNG img;
    img.readFromFile("images/stanleySquare.png");

    toqutree t1(img,9);
    toqutree t1copy(t1);

    PNG out = t1copy.render();
    out.writeToFile("images/out-stanleySquare.png");
    cout << t1.size() << endl;


    REQUIRE(out==img);
}

TEST_CASE("toqutree::basic prune","[weight=1][part=toqutree]"){
    PNG img;
    img.readFromFile("images/pearl.png");
    
    toqutree t1(img,9);

    t1.prune(0.05);
    PNG result = t1.render();

    result.writeToFile("images/out-pearl.png");
    PNG expected; expected.readFromFile("images/adaPrune.05.png");
    //expected.writeToFile("images/out-adaPrune.png");
    result.convert();

    REQUIRE(expected==result);
}


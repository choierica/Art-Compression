
/**
 *
 * toqutree (pa3)
 * significant modification of a quadtree .
 * toqutree.cpp
 * This file will be used for grading.
 *
 */

#include "toqutree.h"

toqutree::Node::Node(pair<int, int> ctr, int dim, HSLAPixel a)
        : center(ctr), dimension(dim), avg(a), NW(NULL), NE(NULL), SE(NULL), SW(NULL) {}

toqutree::~toqutree() {
    clear(root);
}

toqutree::toqutree(const toqutree &other) {
    root = copy(other.root);
    originalImg = other.originalImg;
}

toqutree &toqutree::operator=(const toqutree &rhs) {
    if (this != &rhs) {
        clear(root);
        root = copy(rhs.root);
    }
    return *this;
}

/* This constructor grabs the 2^k x 2^k sub-image centered */
/* in imIn and uses it to build a quadtree. It may assume  */
/* that imIn is large enough to contain an image of that size. */
toqutree::toqutree(PNG &imIn, int k) {
    root = buildTree(&imIn, k);
    originalImg = imIn;
}

int toqutree::size() {
    return size(root);
}

int toqutree::size(toqutree::Node *pNode) {
    if (pNode != nullptr) {
        return 1 + size(pNode->NW) + size(pNode->NE) + size(pNode->SW) + size(pNode->SE);
    }
    return 0;
}

// Note that you will want to practice careful memory use
// In this function. We pass the dynamically allocated image
// via pointer so that it may be released after it is used.
// similarly, at each level of the tree you will want to
// declare a dynamically allocated stats object, and free it
// once you've used it to choose a split point, and calculate
// an average.
toqutree::Node *toqutree::buildTree(PNG *im, int k) {
    if (k == -1) {
        return nullptr;
    }

    auto *s = new stats(*im);
    auto rootRectLength = (unsigned int) pow(2, k);
    auto subRectLength = (unsigned int) pow(2, k - 1);

    pair<int, int> splittingPoint = getSplittingPoint(*s, k);
    pair<int, int> ul = make_pair(0, 0);
    pair<int, int> lr = make_pair(rootRectLength - 1, rootRectLength - 1);

    HSLAPixel avgPixel = s->getAvg(ul, lr);
    Node *retNode = new Node(splittingPoint, k, avgPixel);

    delete(s);
    s = nullptr;

    pair<int, int> seUL = retNode->center;
    int modX = (seUL.first + subRectLength) % rootRectLength;
    int modY = (seUL.second + subRectLength) % rootRectLength;
    pair<int, int> swUL = make_pair(modX, seUL.second);
    pair<int, int> neUL = make_pair(seUL.first, modY);
    pair<int, int> nwUL = make_pair(modX, modY);

    PNG sePNG = getPNG(*im, seUL, k);
    PNG swPNG = getPNG(*im, swUL, k);
    PNG nePNG = getPNG(*im, neUL, k);
    PNG nwPNG = getPNG(*im, nwUL, k);

    retNode->NW = buildTree(&nwPNG, k - 1);
    retNode->NE = buildTree(&nePNG, k - 1);
    retNode->SW = buildTree(&swPNG, k - 1);
    retNode->SE = buildTree(&sePNG, k - 1);

    // if k = 3, 2^3 * 2^3 = 8*8 images.
    // set : center, dimension(k?), avg, NW, NE, SE, SW
    // center: set the optimal splitting position = min avg entropy
    // dimension: node represents a square
    return retNode;
}

pair<int, int> toqutree::getSplittingPoint(stats &s, int k) {
    auto d = (unsigned int) pow(2, k);
    auto basePoint = (d / 2) - (d / 4);
    auto subRectLength = (unsigned int) pow(2, k - 1);

    pair<int, int> baseUL = make_pair(basePoint, basePoint);
    pair<int, int> baseLR = make_pair(basePoint + subRectLength - 1, basePoint + subRectLength - 1);

    pair<int, int> retVal = baseUL;

    double avgEntropy;
    double minEntropy = getAvgEntropy(s, baseUL.first, baseUL.second, k);

    for (int i = baseUL.first; i < baseLR.first; i++) {
        for (int j = baseUL.second; j < baseLR.second; j++) {
            avgEntropy = getAvgEntropy(s, i, j, k);
            if (avgEntropy < minEntropy) {
                minEntropy = avgEntropy;
                retVal = make_pair(i, j);
            }
        }
    }
    return retVal;
}

double toqutree::getAvgEntropy(stats &s, int i, int j, int k) {
    auto rootRectLength = (unsigned int) pow(2, k);
    auto subRectLength = (unsigned int) pow(2, k - 1);

    pair<int, int> seUL = make_pair(i, j);
    pair<int, int> mod = make_pair((seUL.first + subRectLength) % rootRectLength,
                                   (seUL.second + subRectLength) % rootRectLength);
    pair<int, int> swUL = make_pair(mod.first, seUL.second);
    pair<int, int> neUL = make_pair(seUL.first, mod.second);
    pair<int, int> nwUL = mod;

    double seEntropy = getRectEntropy(s, seUL, k);
    double swEntropy = getRectEntropy(s, swUL, k);
    double neEntropy = getRectEntropy(s, neUL, k);
    double nwEntropy = getRectEntropy(s, nwUL, k);;

    double avgEntropy = (seEntropy + swEntropy + neEntropy + nwEntropy) / 4;
    return avgEntropy;
}

double toqutree::getRectEntropy(stats &s, pair<int, int> seUL, int k) {
    auto rootRectLength = (unsigned int) pow(2, k);
    auto subRectLength = (unsigned int) pow(2, k - 1);

    double rectEntropy;
    int lastIndex = rootRectLength - 1;
    pair<int, int> seLR = make_pair(((seUL.first + subRectLength) - 1) % rootRectLength,
                                    ((seUL.second + subRectLength) - 1) % rootRectLength);

    if ((seLR.first > seUL.first) && (seLR.second > seUL.second)) {
        rectEntropy = s.entropy(seUL, seLR);

    } else if ((seLR.first > seUL.first) && (seLR.second < seUL.second)) {
        pair<int, int> upperUL = make_pair(seUL.first, 0);
        pair<int, int> lowerLR = make_pair(seLR.first, lastIndex);

        double upperEntropy = s.entropy(upperUL, seLR);
        double lowerEntropy = s.entropy(seUL, lowerLR);

        rectEntropy = upperEntropy + lowerEntropy;

    } else if ((seLR.second > seUL.second) && (seLR.first < seUL.first)) {
        pair<int, int> leftUL = make_pair(0, seLR.second);
        pair<int, int> rightLR = make_pair(lastIndex, seLR.second);

        double leftEntropy = s.entropy(leftUL, seLR);
        double rightEntropy = s.entropy(seUL, rightLR);

        rectEntropy = leftEntropy + rightEntropy;
    } else {
        pair<int, int> lowerRightLR = make_pair(lastIndex, lastIndex);
        pair<int, int> lowerLeftUL = make_pair(0, seUL.second);
        pair<int, int> lowerLeftLR = make_pair(seLR.first, lastIndex);
        pair<int, int> upperLeftUL = make_pair(0, 0);
        pair<int, int> upperRightUL = make_pair(seUL.first, 0);
        pair<int, int> upperRightLR = make_pair(lastIndex, seLR.second);

        double lowerRightEntropy = s.entropy(seUL, lowerRightLR);
        double lowerLeftEntropy = s.entropy(lowerLeftUL, lowerLeftLR);
        double upperLeftEntropy = s.entropy(upperLeftUL, seLR);
        double upperRightEntropy = s.entropy(upperRightUL, upperRightLR);

        rectEntropy = lowerRightEntropy + lowerLeftEntropy + upperRightEntropy + upperLeftEntropy;
    }

    return rectEntropy;
}

PNG toqutree::render() {
    // if originalImage's width or height is bigger than rootRectLength,
    // then points that out of bounds should be keep the original image.
    // How to calculate the boundary?
    PNG retPNG = originalImg;
    for (unsigned int y = 0; y < originalImg.height(); y++) {
        for (unsigned int x = 0; x < originalImg.width(); x++) {
            //check the bounds
            if (isInTheBound(x, y)) {
                HSLAPixel* testPixel = retPNG.getPixel(x, y);
                HSLAPixel* test2Pixel = getPixelFromTree(x, y, root);
                *(retPNG.getPixel(x, y)) = *getPixelFromTree(x, y, root);
            }
        }
    }
    return retPNG;
}

HSLAPixel *toqutree::getPixelFromTree(unsigned int x, unsigned int y, toqutree::Node *curr) {
    auto *retPixel = new HSLAPixel();
    if (curr->NW == nullptr && curr->NE == nullptr &&
        curr->SW == nullptr && curr->SE == nullptr) {
        *retPixel = curr->avg;
        return retPixel;
    }

    enum QUADRANT {NW, NE, SW, SE};
    QUADRANT quad;
    int distX = curr->center.first - x;
    int distY = curr->center.second - y;
    auto rootRectLength = (unsigned int) pow(2, curr->dimension);
    auto subRectLength = (unsigned int) pow(2, curr->dimension - 1);
    pair<unsigned int, unsigned int> seUL = curr->center;
    int modX = (seUL.first + subRectLength) % rootRectLength;
    int modY = (seUL.second + subRectLength) % rootRectLength;
    pair<unsigned int, unsigned int> swUL = make_pair(modX, seUL.second);
    pair<unsigned int, unsigned int> neUL = make_pair(seUL.first, modY);
    pair<unsigned int, unsigned int> nwUL = make_pair(modX, modY);
    pair<unsigned int, unsigned int> paramXY;

    // calc the distance of x/y from the splitting point.
    // If x/y is abs(subrectlength), then the quad will be changed.

    quad = (QUADRANT) getQuadrant(distX, distY, subRectLength);
    // relative coordinate of x, y from the UL of the quadrants.
    switch (quad) {
        case NW:
            paramXY = make_pair(getParamXY(x, nwUL.first, subRectLength), getParamXY(y, nwUL.second, subRectLength));
            return getPixelFromTree(paramXY.first, paramXY.second, curr->NW);
        case NE:
            paramXY = make_pair(getParamXY(x, neUL.first, subRectLength), getParamXY(y, neUL.second, subRectLength));
            return getPixelFromTree(paramXY.first, paramXY.second, curr->NE);
        case SW:
            paramXY = make_pair(getParamXY(x, swUL.first, subRectLength), getParamXY(y, swUL.second, subRectLength));
            return getPixelFromTree(paramXY.first, paramXY.second, curr->SW);
        case SE:
            paramXY = make_pair(getParamXY(x, seUL.first, subRectLength), getParamXY(y, seUL.second, subRectLength));
            return getPixelFromTree(paramXY.first, paramXY.second, curr->SE);
    }
}


int toqutree::getParamXY(unsigned int coord, unsigned int ul, unsigned int length) {
    if ((ul - coord) > length) {
        return ul + coord;
    } else {
        return ul - coord;
    }
}

int toqutree::getQuadrant(int x, int y, unsigned int rectLength) {
    if (abs(x) >= rectLength && abs(y) >= rectLength) {
        return 0; // NW
    } else if (abs(x) < rectLength && abs(y) >= rectLength) {
        return 1; // NE
    } else if (abs(x) >= rectLength && abs(y) < rectLength) {
        return 2; // SW
    } else {
        return 3; // SE
    }
}

bool toqutree::isInTheBound(int x, int y) {
    bool retVal = false;
    auto rootRectLength = (unsigned int) pow(2, root->dimension);
    pair<int, int> rootRectUL = make_pair((originalImg.width() / 2) - (rootRectLength / 2),
                                          (originalImg.height() / 2) - (rootRectLength / 2));
    pair<int, int> rootRectLR = make_pair(rootRectUL.first + rootRectLength,
                                          rootRectUL.second + rootRectLength);

    if (x >= rootRectUL.first && x <= rootRectLR.first &&
        y >= rootRectUL.second && y <= rootRectLR.second) {
        retVal = true;
    }

    return retVal;
}

/* oops, i left the immakeplementation of this one in the file! */
void toqutree::prune(double tol) {
    prune(root, tol);
}

void toqutree::prune(toqutree::Node *&a, double &tol) { //test
    if (a == nullptr) {
        return;
    }
    HSLAPixel avgRoot = a->avg;
    Node *nwChild = a->NW;
    Node *neChild = a->NE;
    Node *swChild = a->SW;
    Node *seChild = a->SE;

    if (withinTol(seChild, tol, avgRoot) && withinTol(neChild, tol, avgRoot)
        && withinTol(nwChild, tol, avgRoot) && withinTol(swChild, tol, avgRoot)) {
        clear(a->NW);
        clear(a->NE);
        clear(a->SW);
        clear(a->SE);
    } else {
        prune(nwChild, tol);
        prune(neChild, tol);
        prune(swChild, tol);
        prune(seChild, tol);
    }
}

bool toqutree::withinTol(toqutree::Node *pNode, double &tol, HSLAPixel avgPixel) {
    for (int i = 0; i < pNode->dimension; i++)
        for (int j = 0; j < pNode->dimension; j++) {
            if (avgPixel.dist(pNode->avg) > tol) {
                return false;
            }
        }
    return true;
}

/* called by destructor and assignment operator*/
void toqutree::clear(Node *&curr) {
    if (curr == nullptr) {
        return;
    }
    clear(curr->NW);
    clear(curr->NE);
    clear(curr->SW);
    clear(curr->SE);

    delete curr;
    curr = nullptr;
}

/* called by assignment operator and copy constructor */
toqutree::Node *toqutree::copy(const Node *other) {
    if (other == nullptr) {
        return nullptr;
    }
    Node *newNode = new Node(other->center, other->dimension, other->avg);
    newNode->NW = copy(other->NW);
    newNode->NE = copy(other->NE);
    newNode->SW = copy(other->SW);
    newNode->SE = copy(other->SE);

    return newNode;
}

PNG toqutree::getPNG(PNG &im, pair<int, int> ul, int k) {
    auto rootRectLength = (unsigned int) pow(2, k);
    PNG retPNG(rootRectLength, rootRectLength);

    for (unsigned int i = 0; i < rootRectLength; i++) {
        for (unsigned int j = 0; j < rootRectLength; j++) {
            unsigned int xCoord = (ul.first + j) % rootRectLength;
            unsigned int yCoord = (ul.second + i) % rootRectLength;
            HSLAPixel *pixel = retPNG.getPixel(i, j);
            *pixel = *im.getPixel(xCoord, yCoord);
        }
    }
    return retPNG;


}


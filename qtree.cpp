/**
 * @file qtree.cpp
 * @description student implementation of QTree class used for storing image data
 *              CPSC 221 PA3
 *
 *              SUBMIT THIS FILE
 */

#include "qtree.h"
#include <stack>

/**
 * Constructor that builds a QTree out of the given PNG.
 * Every leaf in the tree corresponds to a pixel in the PNG.
 * Every non-leaf node corresponds to a rectangle of pixels
 * in the original PNG, represented by an (x,y) pair for the
 * upper left corner of the rectangle and an (x,y) pair for
 * lower right corner of the rectangle. In addition, the Node
 * stores a pixel representing the average color over the
 * rectangle.
 *
 * The average color for each node in your implementation MUST
 * be determined in constant time. HINT: this will lead to nodes
 * at shallower levels of the tree to accumulate some error in their
 * average color value, but we will accept this consequence in
 * exchange for faster tree construction.
 * Note that we will be looking for specific color values in our
 * autograder, so if you instead perform a slow but accurate
 * average color computation, you will likely fail the test cases!
 *
 * Every node's children correspond to a partition of the
 * node's rectangle into (up to) four smaller rectangles. The node's
 * rectangle is split evenly (or as close to evenly as possible)
 * along both horizontal and vertical axes. If an even split along
 * the vertical axis is not possible, the extra line will be included
 * in the left side; If an even split along the horizontal axis is not
 * possible, the extra line will be included in the upper side.
 * If a single-pixel-wide rectangle needs to be split, the NE and SE children
 * will be null; likewise if a single-pixel-tall rectangle needs to be split,
 * the SW and SE children will be null.
 *
 * In this way, each of the children's rectangles together will have coordinates
 * that when combined, completely cover the original rectangle's image
 * region and do not overlap.
 */

QTree::QTree(const PNG& imIn) {
	height = imIn.height();
	width = imIn.width();

	root = BuildNode(imIn, {0,0}, {width - 1, height - 1});
}

/**
 * Overloaded assignment operator for QTrees.
 * Part of the Big Three that we must 
 * define because the class
 * allocates dynamic memory. This depends on your implementation
 * of the copy and clear funtions.
 *
 * @param rhs The right hand side of the assignment statement.
 */

QTree& QTree::operator=(const QTree& rhs) {
    if (this != &rhs) {
        Clear();
        Copy(rhs);
    }
    return *this;
}

/**
 * Render returns a PNG image consisting of the pixels
 * stored in the tree. may be used on pruned trees. Draws
 * every leaf node's rectangle onto a PNG canvas using the
 * average color stored in the node.
 *
 * For up-scaled images, no color interpolation will be done;
 * each rectangle is fully rendered into a larger rectangular region.
 *
 * @param scale multiplier for each horizontal/vertical dimension
 * @pre scale > 0
 */

PNG QTree::Render(unsigned int scale) const {
    stack<Node*> nodeStack;
    PNG image;
    image.resize(width * scale, height * scale);

    nodeStack.push(root);

    while (!nodeStack.empty()) {
        Node* node = nodeStack.top();
        nodeStack.pop();

        if (node->NE == nullptr && node->NW == nullptr && node->SE == nullptr && node->SW == nullptr) {
            unsigned int startX = node->upLeft.first * scale;
            unsigned int startY = node->upLeft.second * scale;
            unsigned int endX = (node->lowRight.first + 1) * scale;
            unsigned int endY = (node->lowRight.second + 1) * scale;

            for (unsigned int i = startX; i < endX && i < image.width(); i++) {
                for (unsigned int j = startY; j < endY && j < image.height(); j++) {
                    *image.getPixel(i, j) = node->avg;
                }
            }
        } else {
            if (node->NW) nodeStack.push(node->NW);
            if (node->NE) nodeStack.push(node->NE);
            if (node->SW) nodeStack.push(node->SW);
            if (node->SE) nodeStack.push(node->SE);
        }
    }

    return image;
}


/**
 *  Prune function trims subtrees as high as possible in the tree.
 *  A subtree is pruned (cleared) if all of the subtree's leaves are within
 *  tolerance of the average color stored in the root of the subtree.
 *  NOTE - you may use the distanceTo function found in RGBAPixel.h
 *  Pruning criteria should be evaluated on the original tree, not
 *  on any pruned subtree. (we only expect that trees would be pruned once.)
 *
 * You may want a recursive helper function for this one.
 *
 * @param tolerance maximum RGBA distance to qualify for pruning
 * @pre this tree has not previously been pruned, nor is copied from a previously pruned tree.
 */
void QTree::Prune(double tolerance) {
    PruneHelper(root, tolerance);
}

/**
 *  FlipHorizontal rearranges the contents of the tree, so that
 *  its rendered image will appear mirrored across a vertical axis.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  After flipping, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel wide rectangles have
 *  null eastern children
 *  (i.e. after flipping, a node's NW and SW pointers may be null, but
 *  have non-null NE and SE)
 *
 *  You may want a recursive helper function for this one.
 */
void QTree::FlipHorizontal() {
    FlipHorizontalHelper(root, width);
}

/**
 *  RotateCCW rearranges the contents of the tree, so that its
 *  rendered image will appear rotated by 90 degrees counter-clockwise.
 *  This may be called on a previously pruned/flipped/rotated tree.
 *
 *  Note that this may alter the dimensions of the rendered image, relative
 *  to its original dimensions.
 *
 *  After rotation, the NW/NE/SW/SE pointers must map to what will be
 *  physically rendered in the respective NW/NE/SW/SE corners, but it
 *  is no longer necessary to ensure that 1-pixel tall or wide rectangles
 *  have null eastern or southern children
 *  (i.e. after rotation, a node's NW and NE pointers may be null, but have
 *  non-null SW and SE, or it may have null NW/SW but non-null NE/SE)
 *
 *  You may want a recursive helper function for this one.
 */
void QTree::RotateCCW() {
	//the new x coordinate is the height of the image minus the old y coordinate minus one, and the new y coordinate is the old x coordinate.
    unsigned int originalHeight = height; 
    swap(height, width);
    RotateHelper(root, originalHeight);
}

/**
 * Destroys all dynamically allocated memory associated with the
 * current QTree object. Complete for PA3.
 * You may want a recursive helper function for this one.
 */
void QTree:: Clear() {
	ClearHelper(root);
}

/**
 * Copies the parameter other QTree into the current QTree.
 * Does not free any memory. Called by copy constructor and operator=.
 * You may want a recursive helper function for this one.
 * @param other The QTree to be copied.
 */
void QTree::Copy(const QTree& other) {
	root = CopyHelper(other.root);
	width = other.width;
    height = other.height;
}

/**
 * Private helper function for the constructor. Recursively builds
 * the tree according to the specification of the constructor.
 * @param img reference to the original input image.
 * @param ul upper left point of current node's rectangle.
 * @param lr lower right point of current node's rectangle.
 */
Node* QTree::BuildNode(const PNG& img, pair<unsigned int, unsigned int> ul, pair<unsigned int, unsigned int> lr) {
    if (ul == lr) return new Node(ul, lr, *img.getPixel(ul.first, ul.second));
    
    unsigned int verticalLine = ul.first + (lr.first - ul.first) / 2;
    unsigned int horizontalLine = ul.second + (lr.second - ul.second) / 2;

    Node* node = new Node(ul, lr, RGBAPixel(0,0,0));

    if (lr.first - ul.first == 0) {
        node->NE = node->SE = nullptr;
        node->NW = BuildNode(img, ul, {ul.first, horizontalLine});
        node->SW = BuildNode(img, {ul.first, horizontalLine + 1}, lr);
    } else if (lr.second - ul.second == 0) {
        node->SW = node->SE = nullptr;
        node->NW = BuildNode(img, ul, {verticalLine, ul.second});
        node->NE = BuildNode(img, {verticalLine + 1, ul.second}, lr);
    } else {
        node->NW = BuildNode(img, ul, {verticalLine, horizontalLine}); 
        node->NE = BuildNode(img, {verticalLine + 1, ul.second}, {lr.first, horizontalLine});
        node->SW = BuildNode(img, {ul.first, horizontalLine + 1}, {verticalLine, lr.second}); 
        node->SE = BuildNode(img, {verticalLine + 1, horizontalLine + 1}, lr); 
    }

    node->avg = AverageColor(node->NW, node->NE, node->SW, node->SE);
    return node;	
}


/*********************************************************/
/*** IMPLEMENT YOUR OWN PRIVATE MEMBER FUNCTIONS BELOW ***/
/*********************************************************/


RGBAPixel QTree::AverageColor(Node *nw, Node *ne, Node *sw, Node *se) {
    unsigned long long totalArea = 0;
    unsigned long long red = 0;
    unsigned long long green = 0;
    unsigned long long blue = 0;

    auto addWeightedColor = [&](Node* node) {
        if (node) {
            unsigned int nodeWidth = node->lowRight.first - node->upLeft.first + 1;
            unsigned int nodeHeight = node->lowRight.second - node->upLeft.second + 1;
            unsigned int nodeArea = nodeWidth * nodeHeight;

            red += nodeArea * node->avg.r;
            green += nodeArea * node->avg.g;
            blue += nodeArea * node->avg.b;
            totalArea += nodeArea;
        }
    };

    addWeightedColor(nw);
    addWeightedColor(ne);
    addWeightedColor(sw);
    addWeightedColor(se);

    if (totalArea > 0) {
        red /= totalArea;
        green /= totalArea;
        blue /= totalArea;
    }

    return RGBAPixel(static_cast<unsigned char>(red), static_cast<unsigned char>(green), static_cast<unsigned char>(blue));
}


void QTree::FlipHorizontalHelper(Node* node, unsigned int width) {
    if (node == nullptr) return;

    swap(node->NW, node->NE);
    swap(node->SW, node->SE);

    FlipHorizontalHelper(node->NW, width);
    FlipHorizontalHelper(node->NE, width);
    FlipHorizontalHelper(node->SW, width);
    FlipHorizontalHelper(node->SE, width);

    unsigned int newLeftX = width - 1 - node->lowRight.first;
    unsigned int newRightX = width - 1 - node->upLeft.first;

    node->upLeft.first = newLeftX;
    node->lowRight.first = newRightX;
}


void QTree::ClearHelper(Node*& node) {
    if (node == nullptr) return;

    ClearHelper(node->NW);
    ClearHelper(node->NE);
    ClearHelper(node->SE);
    ClearHelper(node->SW);

    delete node;
    node = nullptr;
}

Node* QTree::CopyHelper(Node* node) {
    if (node == nullptr) return nullptr;

    Node* newNode = new Node(node->upLeft, node->lowRight, node->avg);

    newNode->NW = CopyHelper(node->NW);
    newNode->NE = CopyHelper(node->NE);
    newNode->SW = CopyHelper(node->SW);
    newNode->SE = CopyHelper(node->SE);

    return newNode;
}

void QTree::RotateHelper(Node* node, unsigned int originalHeight) {
    if (node == nullptr) return;

    RotateHelper(node->NW, originalHeight);
    RotateHelper(node->NE, originalHeight);
    RotateHelper(node->SW, originalHeight);
    RotateHelper(node->SE, originalHeight);

    Node* tempNW = node->NW;
    Node* tempNE = node->NE;
    Node* tempSE = node->SE;
    Node* tempSW = node->SW;

    node->NW = tempNE;
    node->NE = tempSE;
    node->SE = tempSW;
    node->SW = tempNW;


    pair<unsigned int, unsigned int> newUpLeft(node->upLeft.second, height - node->lowRight.first - 1);
    pair<unsigned int, unsigned int> newLowRight(node->lowRight.second, height - node->upLeft.first - 1);

    node->upLeft = newUpLeft;
    node->lowRight = newLowRight;
}

void QTree::clearChildren(Node* node) {
    if (node == nullptr) return;

    clearDelete(node->NW);
    clearDelete(node->NE);
    clearDelete(node->SW);
    clearDelete(node->SE);
}

void QTree::clearDelete(Node*& child) {
    if (child != nullptr) {
        clearChildren(child);
        delete child;
        child = nullptr;
    }
}

void QTree::PruneHelper(Node* node, double tolerance) {
    if (node == nullptr) return;

    if (node->NW == nullptr && node->NE == nullptr && node->SW == nullptr && node->SE == nullptr) return;

    if (WithinTolerance(node, node->avg, tolerance)) {
        clearChildren(node);
        return;
    }

    PruneHelper(node->NW, tolerance);
    PruneHelper(node->NE, tolerance);
    PruneHelper(node->SW, tolerance);
    PruneHelper(node->SE, tolerance);
}


bool QTree::WithinTolerance(Node* node, RGBAPixel avg, double tolerance) {
    if (node == nullptr) return true;

    if (node->NW == nullptr && node->NE == nullptr && node->SW == nullptr && node->SE == nullptr) return node->avg.distanceTo(avg) <= tolerance;

    return WithinTolerance(node->NW, avg, tolerance) &&
           WithinTolerance(node->NE, avg, tolerance) &&
           WithinTolerance(node->SW, avg, tolerance) &&
           WithinTolerance(node->SE, avg, tolerance);
}
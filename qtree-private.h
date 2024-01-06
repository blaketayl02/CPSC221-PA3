/**
 * @file qtree-private.h
 * @description student declaration of private QTree functions
 *              CPSC 221 PA3
 *
 *              SUBMIT THIS FILE.
 * 
 *				Simply declare your function prototypes here.
 *              No other scaffolding is necessary.
 */

// begin your declarations below

Node* CopyHelper(Node* node);

void ClearHelper(Node*& node);

void RotateHelper(Node* node, unsigned int originalHeight);

void FlipHorizontalHelper(Node* node, unsigned int width);

void clearDelete(Node*& child);

void clearChildren(Node* node);

bool WithinTolerance(Node* node, RGBAPixel avg, double tolerance);

void PruneHelper(Node* node, double tolerance);

RGBAPixel AverageColor(Node *nw, Node *ne, Node *sw, Node *se);
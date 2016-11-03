/**
    Copyright (c) 2016 <PeiXu xuxx0884@umn.edu>

    Permission is hereby granted, free of charge, to any person obtaining a copy
       of this software and associated documentation files (the "Software"), to
       deal in the Software without restriction, including without limitation
       the rights to use, copy, modify, merge, publish, distribute, sublicense,
       and/or sell copies of the Software, and to permit persons to whom the
       Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
       all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
       IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
       FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
       THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
       OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
       ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
       OTHER DEALINGS IN THE SOFTWARE.
 */

/**
    File: hashTree.h
    Purpose: Header file for the implementation of Hash Tree used in Apriori
       algorithm
    @author Pei Xu
    @version 0.9 10/24/2016
 */

#ifndef HASHTREE_H
#define HASHTREE_H

#include <map>
#include <list>
#include <queue>
#include <algorithm>
#include <mutex>

#include "itemTree.h"

namespace hashTree {
// struct Dataset
// {
//     std::list<int>data;
//     int unsigned  count;
//     Dataset(std::list<int>data) : data(data), count(0) {}
// };

struct Node
{
    int id;

    // std::list<Dataset *> dataset;
    std::list<itemTree::Node *>dataset;
    int                        level;
    std::map<int, Node *>      children;

    Node() : id(0), level(0) {}

    // Node(int id, std::list<Dataset *>data, int unsigned level) :
    // id(id), dataset(data), level(level) {
    // }
    Node(int id, std::list<itemTree::Node *>data, int unsigned level) :
        id(id), dataset(data), level(level) {}

    ~Node();
};

class HashTree {
public:

    HashTree(int data_size,
             int hash_range,
             int max_leafsize);
    ~HashTree();
    bool              insert(itemTree::Node *dataset);
    itemTree::Node  * find(const std::list<int>& data,
                           const int           & auto_count = 0);
    bool              findSubsetCountLast(std::list<int>dataset,
                                          const int   & count);
    inline static int calComboNum(const int& n,
                                  const int& k);
    int               getDataSize();

protected:

    int _hashFunc(const int& v);
    std::mutex _count_lock;

private:

    int unsigned _hash_range;
    int unsigned _max_leafsize;
    int   _last_node_id;
    Node *_tree_origin;
    int   _data_size;
    int _genNodeId();
};
}

#endif // ifndef HASHTREE_H

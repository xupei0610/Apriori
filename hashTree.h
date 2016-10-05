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
    @version 1.0 10/3/2016
 */

#ifndef HASHTREE_H
#define HASHTREE_H
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <tuple>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>

#include <condition_variable>

typedef int              Item;
typedef std::vector<Item>Data;

struct Dataset
{
    Data         data;
    int unsigned count;
    Dataset(Data data) : data(data), count(0) {}
};

struct Node
{
    std::list<Dataset *> dataset;
    int unsigned         level;
    std::map<int, Node *>children;

    Node() : level(0) {} // Used for building the original node

    Node(std::list<Dataset *>data, int unsigned level) :
        dataset(data), level(level) {}

    ~Node();
};

class HashTree {
public:

    HashTree(int data_size,
             int hash_range,
             int max_leafsize);
    ~HashTree();
    std::mutex count_lock;

    bool     insert(Dataset *dataset,
                    Node    *parent_node = NULL);
    Dataset* find(const Data& data,
                  const bool& auto_count = false);
    bool     findSubsetOf(Data      & data,
                          const bool& auto_count);

    // data must be sorted ascendingly, not repeated.

protected:

    int hashFunction(const int& v);
    Node *tree_origin; // Use pointer in order to keep the type the same with
    // its children

private:

    int unsigned hash_range;
    int unsigned max_leafsize;
    int unsigned data_size;
};

#endif // ifndef HASHTREE_H

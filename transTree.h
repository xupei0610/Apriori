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
 File: transTree.h
 algorithm
 @author Pei Xu
 @version 0.9 10/24/2016
 */

#ifndef TRANSTREE_H
#define TRANSTREE_H

#include <list>
#include <stack>
#include <atomic>
#include <mutex>
#include <thread>

#include "hashTree.h"

namespace transTree {
const int MAX_THREADS = 60;

struct Node {
    int                  data;
    std::list<int>       dataset;
    int                  count;
    int                  level;
    int                  visited;
    Node                *parent;
    Node                *left;
    Node                *right;
    bool                 abandoned;
    std::map<int, Node *>children;
    std::list<int>       trans;

    // Used for the root of trans tree or the identifier of the header table
    Node() : count(0), level(0), visited(0), parent(nullptr), left(nullptr),
        right(nullptr), abandoned(true) {}

    Node(int  data, Node *parent);
    ~Node();
};

class TransTree {
public:

    TransTree();
    ~TransTree();
    void count(hashTree::HashTree *itemset_hashtree);
    void addTransaction(std::list<int>trans_collection,
                        const int   & id);

private:

    bool _update_lock;
    std::map<int, Node *> _header_table;
    Node *_tree_origin;
    static std::atomic<bool> _boom;
    static std::mutex _worklock;
    static std::stack<Node *>  _workbench;
    static hashTree::HashTree *_itemset_hashtree;
    static std::list<int> _passed;
    static void _counter(const int tid);

};
}

#endif // ifndef TRANSTREE_H

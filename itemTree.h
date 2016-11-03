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
 File: itemTree.h
 algorithm
 @author Pei Xu
 @version 0.9 10/24/2016
 */


#ifndef ITEMTREE_H
#define ITEMTREE_H

#include <list>
#include <map>
#include <vector>
#include <queue>
#include <list>
#include <thread>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <mutex>

namespace itemTree {
        const int EMPTY_DATA = -1;
        const int MAX_THREADS = 60;
        struct Node
        {
                int data;
                std::list<int>       dataset;
                int count;
                int level;
                Node                *parent;
                std::map<int, Node *>children;
                std::list<int>       trans;
                Node() : data(EMPTY_DATA), count(0), level(0), parent(nullptr)  {
                }

                Node(int data,
                     Node         *parent);
                Node(int data,
                     Node         *parent,
                     std::list<int>trans);
                ~Node();
        };

        class ItemTree {
public:

                ItemTree();
                ~ItemTree();
                int             plant(const int& min_sup,
                                      const std::map<int, std::list<int>> & item_table);
                std::list<Node *>grow();

                // std::list<std::list<int> >collect(const bool& projected = false);
                int              getCurrentLevel();

private:

                Node *_tree_origin;
                bool _insertlock;
                // std::map<int, int> _proj_map;  No projection used yet

                static int _min_sup;
                static int _level;
                static std::atomic<bool>  _boom;
                static std::mutex _worklock;
                static std::mutex _worknotelock;
                static std::queue<Node *> _workbench;
                static std::list<Node *> _obsoleted;
                static std::list<Node *> _candidates;

                void _addItem(const int& id,
                              const int& count);
                static void _sprout(const int & tid);
        };
}


#endif // ifndef ITEMTREE_H

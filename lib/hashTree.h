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
 @version 0.9 10/7/2016
 */

#ifndef HASHTREE_H
#define HASHTREE_H

#include <map>
#include <list>
#include <queue>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>
#include "item.h"
#include "trans.h"

#include <iostream>

namespace hashTree {
    
    struct Node
    {
        int id;
        int level;
        std::list<item::Node *> dataset;
        int data_num = 0;
        std::map<int, Node *>      children;
        
        Node() : id(-1), level(0), data_num(0) {}
        
        Node(const int & id, const int & level, item::Node * data) : id(id), level(level), data_num(1)
        {
            this->dataset.push_back(data);
        }

        ~Node(){
            for (auto & c: this->children) delete c.second;
        }
        
        void addData(item::Node * data)
        {
            this->dataset.push_back(data);
            this->data_num++;
        }
        
        void clearDataset()
        {
            this->dataset.clear();
            this->data_num = 0;
        }
    };


    class HashTree {
    public:
        
        HashTree(const int & hash_range, const int &max_leafsize);
        ~HashTree();
        
        void build(item::Node * cand, const int & datasize);
        int  hashFunc(const int& v);
        Node * getRoot();
        int getDataSize();
        void count(std::list<trans::Trans *> * trans_table);
        bool find(std::list<int> itemset, const int & count = 0);
        bool findSubSetCountLast(const std::list<int> & collection, const int & count);
        static int calComboNum(const int& n, const int& k);
    private:
        
        int _hash_range;
        int _max_leafsize;
        int _data_size;
        Node * _tree_origin;
        int _last_node_id;
        std::vector<item::Item *> * _item_table;
        bool insert(item::Node *dataset);
        void _insert(item::Node * cand);
        int _genNodeID();
        void _count(std::list<int> prefix, const std::list<int>::iterator & key, trans::Trans *, Node * node);

        static std::mutex lock;
        static std::list<trans::Trans *> trans_tb;
        static std::atomic<bool> boom;
        static int data_size;
        static int hfrange;
        static void _multiCount(Node * tree_origin);
    };
}

#endif // ifndef HASHTREE_H

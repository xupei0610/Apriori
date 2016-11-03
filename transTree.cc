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
 File: transTree.cc
 algorithm
 @author Pei Xu
 @version 0.9 10/24/2016
 */


#include "transTree.h"
#include <iostream>

using namespace transTree;

std::atomic<bool>  TransTree::_boom;
std::stack<Node *> TransTree::_workbench;
std::mutex TransTree::_worklock;
hashTree::HashTree *TransTree::_itemset_hashtree;
std::list<int> TransTree::_passed;


Node::Node(int data, Node *parent)
{
    this->data      = data;
    this->parent    = parent;
    this->left      = nullptr;
    this->right     = nullptr;
    this->level     = parent->level + 1;
    this->count     = 0;
    this->abandoned = false;
    this->dataset   = parent->dataset;
    this->dataset.push_back(data);
    this->abandoned = false;
}

Node::~Node()
{
        Node *swap = this->right;

        if (this->left != nullptr)
        {
            this->left->right = std::move(swap);
        }

        if (this->right != nullptr)
        {
            this->right->left = this->left;
        }

        for (auto & c: this->children)
        {
            delete c.second;
        }
}

TransTree::TransTree()
{
    this->_tree_origin = new Node();
    this->_update_lock = false;
}

TransTree::~TransTree()
{
    delete this->_tree_origin;

    for (auto & h : this->_header_table) delete h.second;
}

void TransTree::addTransaction(std::list<int>trans_collection, const int& id)
{
    if (this->_update_lock == true) return;

    Node *container = this->_tree_origin;

    for (auto & it : trans_collection)
    {
        if (container->children.find(it) == container->children.end())
        {
            container->children[it] = new Node(it, container);

            container = container->children[it];

            if (this->_header_table.find(it) == this->_header_table.end())
            {
                this->_header_table[it]        = new Node();
                this->_header_table[it]->right = container;
                this->_header_table[it]->left  = container;
                container->left                = this->_header_table[it];
            }
            else
            {
                this->_header_table[it]->left->right = container;
                container->left =
                    this->_header_table[it]->left;
                this->_header_table[it]->left = container;
            }
        }
        else
        {
            container = container->children[it];
        }
        container->trans.push_back(id);
        container->count++;
    }
}

void TransTree::count(hashTree::HashTree *itemset_hashtree)
{
    this->_update_lock = true;

    int multithread = TransTree::_passed.size() ==
                      0 ?  MAX_THREADS : TransTree::_passed.size();
    TransTree::_boom = false;
        std::list<std::thread> threads;
        TransTree::_itemset_hashtree = itemset_hashtree;

        if (multithread < 1)
        {
            multithread++;
        }
        else if (multithread > MAX_THREADS)
        {
            multithread = MAX_THREADS;
        }

        for (int i = 0; i < multithread; i++)
        {
            threads.emplace_back(TransTree::_counter, i + 1);
        }

        std::stack<Node *>().swap(TransTree::_workbench);

        if (TransTree::_passed.empty())
        {
            for (auto & d : this->_header_table)
            {
                TransTree::_workbench.push(d.second->right);
            }
        }
        else
        {
            for (auto & d : TransTree::_passed)
            {
                TransTree::_workbench.push(this->_header_table[d]->right);
            }
        }

        // std::cout << TransTree::_passed.size() << std::endl;
        
        TransTree::_passed.clear();
        TransTree::_boom = true;
    
        for (auto & th: threads)
        {
            th.join();
        }
}

void TransTree::_counter(const int tid)
{
    while (!TransTree::_boom)
    {
        std::this_thread::yield();
    }

    Node *container;
    bool  res;
    bool  success;
    // int i =0;
    while (true)
    {
        while (!TransTree::_worklock.try_lock()){std::this_thread::yield();}

        if (TransTree::_workbench.empty())
        {
            TransTree::_worklock.unlock();
            break;
        }
        else
        {
            container = TransTree::_workbench.top();
            TransTree::_workbench.pop();
            TransTree::_worklock.unlock();
        }


        if (container == nullptr) continue;

        success = false;

        while (true)
        {
            res = true;

            if (container->abandoned == false)
            {
                if (container->level < TransTree::_itemset_hashtree->getDataSize())
                {
                    res = false;
                }
                else if (container->level ==
                         TransTree::_itemset_hashtree->getDataSize())
                {
                    if (TransTree::_itemset_hashtree->find(container->dataset,
                                                      container->count) ==
                        nullptr)
                    {
                        res = false;
                    }
                }
                else
                {
                    res = TransTree::_itemset_hashtree->findSubsetCountLast(
                        container->dataset,
                        container->count);
                }

                if (res == false)
                {
                    container->abandoned = false;
                }
                else
                {
                    success = true;
                }

            }

            if (container->right == nullptr || container->right == container)
            {
                break;
            }
            container = container->right;

        }

        // if (tid != 0)
        // {
        //     std::cout << "Thread #" << tid << " completed " << ++i <<
        //         " of tasks." << std::endl;
        // }

        if (success == true)
        {
            while(!TransTree::_worklock.try_lock()){}
            TransTree::_passed.push_back(container->data);
            TransTree::_worklock.unlock();
        }
    }
}

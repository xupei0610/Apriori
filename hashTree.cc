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
    File: hashTree.cc
    Purpose: Implementation of Hash Tree used in Apriori algorithm
    @author Pei Xu
    @version 0.9 10/24/2016
 */

#include "hashTree.h"

using namespace hashTree;

Node::~Node()
{
    for (auto & c: this->children) delete c.second;
}

HashTree::HashTree(int data_size,
                   int hash_range,
                   int max_leafsize)
{
    this->_hash_range   = hash_range < 1 ? 1000000 : hash_range;
    this->_max_leafsize = max_leafsize < 1 ? 1 : max_leafsize;
    this->_data_size    = data_size;
    this->_tree_origin  = new Node();
    this->_last_node_id = 0;
}

HashTree::~HashTree()
{
    delete this->_tree_origin;
}

int HashTree::calComboNum(const int& n, const int& k)
{
    int result = 1;

    // TODO: the method of cal combo # may be able to be optimized
    // Brute Force calculation, should be useful enough in most cases
    for (int i = n; i >= n - k + 1; --i)
    {
        result *= i;
    }

    for (int i = k; i > 1; i--)
    {
        result /= i;
    }
    return result;
}

int HashTree::getDataSize()
{
    return this->_data_size;
}

int HashTree::_genNodeId()
{
    return ++this->_last_node_id;
}

int HashTree::_hashFunc(const int& v)
{
    return v % this->_hash_range;
}

itemTree::Node * HashTree::find(const std::list<int>& data, const int& auto_count)
{
    if (data.size() == this->_data_size)
    {
        Node *container = this->_tree_origin;
        int   key;

        for (auto & d : data)
        {
            if (container->dataset.empty())
            {
                key = this->_hashFunc(d);

                if (container->children.find(key) == container->children.end())
                {
                    return nullptr;
                }
                else
                {
                    container = container->children[key];

                    if (d != data.back())
                    {
                        continue;
                    }
                }
            }

            for (auto & da : container->dataset)
            {
                if (da->dataset == data)
                {
                    if (auto_count > 0)
                    {
                        this->_count_lock.lock();
                        da->count += auto_count;
                        this->_count_lock.unlock();
                    }

                    return da;
                }
            }
        }
    }
    return nullptr;
}

bool HashTree::insert(itemTree::Node *dataset)
{
    if (dataset->dataset.size() == this->_data_size)
    {
        Node *container = this->_tree_origin;
        int   hash_key;

        auto it = dataset->dataset.begin();
        auto itn = dataset->dataset.end();
        for (;it!=itn;it++)
        {

            if (container->dataset.empty())
            {
                hash_key = this->_hashFunc(*it);
                if (container->children.find(hash_key) ==
                    container->children.end())
                {
                    container->children[hash_key] =
                        new Node(this->_genNodeId(), std::list<itemTree::Node *>(
                                     { dataset }), container->level + 1);
                    return true;
                }
                else
                {
                    container = container->children[hash_key];
                }
            }
            else
            {
                break;
            }
        }

        if ((container->dataset.size() == this->_data_size) &&
            (this->_data_size > container->level))
        {

            // Insert current node's all dataset into corresponding child
            // nodes respectively
            while(true)
            {
                for (auto itt = container->dataset.begin();
                     itt != container->dataset.end(); itt++)
                {
                    hash_key =
                    this->_hashFunc(*(std::next((*itt)->dataset.begin(),
                                                container->level)));
                    
                    if (container->children.find(hash_key) ==
                        container->children.end())
                    {
                        container->children[hash_key] =
                        new Node(this->_genNodeId(), std::list<itemTree::Node *>(
                                                                                 { *itt }), container->level + 1);
                    }
                    else
                    {
                        container->children[hash_key]->dataset.push_back(*itt);
                    }
                }
                container->dataset.clear();
                
                hash_key = this->_hashFunc(*it);
                
                if (container->children.find(hash_key) == container->children.end())
                {
                    container->children[hash_key] =
                    new Node(this->_genNodeId(),
                             std::list<itemTree::Node *>({ dataset }),
                             container->level + 1);
                    break;
                }
                else
                {
                    container = container->children[hash_key];
                    if ((container->dataset.size() == this->_data_size) &&
                        (this->_data_size > container->level))
                    {
                        it++;
                        continue;
                    }
                    container->dataset.push_back(dataset);
                    break;
                }
            }
        }
        else
        {
            container->dataset.push_back(dataset);
        }
    }
    return false;
}

bool HashTree::findSubsetCountLast(std::list<int>dataset,
                                   const int   & count)
{
    bool result = false;

    Node *container = this->_tree_origin;

    std::queue<std::pair<std::list<int>::iterator, Node *> > unvisited;
    std::pair<std::list<int>::iterator, Node *> current;
    std::list<int> current_data;
    std::list<int>::iterator current_it;
    std::list<int> visited;

    auto it_end = std::next(dataset.end(), 1 - (int)(this->_data_size));

    int hash_key;

    for (auto it = dataset.begin(); it != it_end; it++)
    {
        hash_key = this->_hashFunc(*it);

        if (container->children.find(hash_key) != container->children.end())
        {
            unvisited.push(
                std::make_pair(it, container->children[hash_key])
                );
        }
    }

    std::list<int> max(std::next(dataset.end(), -this->_data_size), dataset.end());
    
    while (!unvisited.empty())
    {
        current = unvisited.front();
        unvisited.pop();
        current_it = std::move(current.first);
        container  = std::move(current.second);
        bool test = true;
        if (container->dataset.empty())
        {
            if (this->_data_size - container->level == 1)
            {
                hash_key = this->_hashFunc(dataset.back());

                if (container->children.find(hash_key) !=
                    container->children.end())
                {
                    container  = container->children[hash_key];
                    current_it = std::next(dataset.end(), -2);
                    test = false;
                }
            }
            else
            {
                it_end = std::next(
                    dataset.end(), 1 + container->level - this->_data_size);

                for (auto it = std::next(current_it, 1); it != it_end; it++)
                {
                    hash_key = this->_hashFunc(*it);

                    if (container->children.find(hash_key) !=
                        container->children.end())
                    {
                        unvisited.push(
                            std::make_pair(it, container->children[hash_key])
                            );
                    }
                }
                continue;
            }
        }

//        // TODO: This is just a simple upper bound; so a bucket still may be
//        // checked wholely.
//        // I have no idea to cal an accurate combo # for the bucket in such
//        // a hash tree.
//        int cnk = HashTree::calComboNum(dataset.size() - 1, this->_data_size - 1);
//        
//        int matched = 0;
        
        if (std::find(visited.begin(), visited.end(),
                      container->id) == visited.end())
        {
            std::list<int> diff;
            visited.push_back(container->id);

            for (auto & da : container->dataset)
            {
                if (da->dataset.back() == dataset.back())
                {
                    diff.clear();

                    std::set_difference(da->dataset.begin(), da->dataset.end(),
                                        dataset.begin(),
                                        dataset.end(), diff.begin());

                    if (diff.empty())
                    {
                        while (!this->_count_lock.try_lock()) {}
                        da->count += count;
                        this->_count_lock.unlock();
                        result = true;
//                        matched++;
//                        if (matched>cnk)
//                        {
//                            std::cout << "cnk " << cnk << "/n:" << dataset.size()-1 << "/k:" << this->_data_size - 1 << std::endl;
//                            break;
//                        }
                    }
                }
            }
        }
    }

    return result;
}

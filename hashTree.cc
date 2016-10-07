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
    @version 0.9 10/7/2016
 */

#include "hashTree.h"

using namespace hashTree;

Node::~Node()
{
    for (std::map<int, Node *>::iterator it = children.begin();
         it != children.end();
         it++) delete it->second;
    children.clear();
}

HashTree::HashTree(int data_size,
                   int hash_range,
                   int max_leafsize)
{
    this->hash_range   = hash_range < 2 ? 2 : hash_range;
    this->max_leafsize = max_leafsize < 2 ? 2 : max_leafsize;

    // TODO: data_size should be greater than 0. There is no meaning when
    // data_size == 0;
    this->data_size = data_size;

    this->tree_origin = new Node();
}

HashTree::~HashTree()
{
    delete this->tree_origin;
}

int HashTree::hashFunction(const int& v)
{
    return v % this->hash_range;
}

Dataset * HashTree::find(const Data& data, const bool& auto_count)
{
    if (data.size() == this->data_size)
    {
        Node *container = this->tree_origin;
        int   key;

        for (auto & d : data)
        {
            if (container->dataset.empty())
            {
                key = this->hashFunction(d);

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
                if (da->data == data)
                {
                    if (auto_count == true)
                    {
                        da->count++;
                    }

                    return da;
                }
            }
        }
    }
    return nullptr;
}

bool HashTree::insert(Dataset *dataset)
{
    if (dataset->data.size() == this->data_size)
    {
        Node *container = this->tree_origin;
        int   hash_key;

        for (auto & d : dataset->data)
        {
            hash_key = this->hashFunction(d);

            if (container->dataset.empty())
            {
                if (container->children.find(hash_key) ==
                    container->children.end())
                {
                    container->children[hash_key] =
                        new Node(std::list<Dataset *>(
                                     { dataset }), container->level + 1);
                    return true;
                }
                else
                {
                    container = container->children[hash_key];

                    if (d != dataset->data.back())
                    {
                        continue;
                    }
                }
            }

            if ((container->dataset.size() == this->max_leafsize) &&
                (this->data_size > container->level))
            {
                container->children[hash_key] =
                    new Node(std::list<Dataset *>({ dataset }),
                             container->level + 1);

                // Insert current node's all dataset into corresponding child
                // nodes respectively

                for (auto it = container->dataset.begin();
                     it != container->dataset.end(); it++)
                {
                    hash_key = hashFunction(((*it)->data)[container->level]);

                    if (container->children.find(hash_key) ==
                        container->children.end())
                    {
                        container->children[hash_key] =
                            new Node(std::list<Dataset *>(
                                         { *it }), container->level + 1);
                    }
                    else
                    {
                        container->children[hash_key]->dataset.push_back(*it);
                    }
                }
                container->dataset.clear();
            }
            else
            {
                container->dataset.push_back(dataset);
            }
        }
    }
    return false;
}

bool HashTree::findSubsetOf(Data       data,
                            const bool& auto_count)
{
    // TODO: no guarantee for the uniqueness and order of data

    Dataset *temp_res;

    if (data.size() < this->data_size)
    {
        return false;
    }

    if (data.size() == this->data_size)
    {
        temp_res = this->find(data, true);

        if (temp_res == nullptr)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    Node *container = this->tree_origin;

    std::queue<std::tuple<Data, Data::iterator, Node *> > unvisited;
    std::tuple<Data, Data::iterator, Node *> current;
    Data current_data;
    Data::iterator current_it;

    auto it_end = std::next(data.end(), 1 - (int)(this->data_size));

    int hash_key;

    for (auto it = data.begin(); it != it_end; it++)
    {
        hash_key = this->hashFunction(*it);

        if (container->children.find(hash_key) != container->children.end())
        {
            unvisited.push(std::make_tuple(Data({ *it }), it,
                                           (container->children)[hash_key]));
        }
    }

    bool result = false;

    while (!unvisited.empty())
    {
        current = unvisited.front();
        unvisited.pop();
        std::tie(current_data, current_it, container) = current;

        if (container->dataset.empty())
        {
            it_end = std::next(data.end(), 1 + container->level - this->data_size);

            for (auto it = std::next(current_it, 1); it != it_end;  it++)
            {
                hash_key = this->hashFunction(*it);

                if (container->children.find(hash_key) !=
                    container->children.end())
                {
                    current_data.push_back(*it);
                    unvisited.push(std::make_tuple(current_data,
                                                   it,
                                                   container->children[
                                                       hash_key]));
                    current_data.pop_back();
                }
            }
            continue;
        }

        auto it = std::next(current_it, 1);
        int ds = current_data.size();
        int k  = this->data_size - ds;
        int n  = std::distance(current_it, data.end())-1;
        std::string bitmask(k, 1);
        bitmask.resize(n, 0);

        do {
            current_data.resize(ds);
            for (int i = 0; i < n; ++i)
            {
                if (bitmask[i]) {
                    current_data.push_back(*std::next(it, i));
                }
            }

            for (auto & da : container->dataset)
            {
                if (da->data == current_data)
                {
                    if (auto_count == true)
                    {
                        this->count_lock.lock();
                        da->count++;
                        this->count_lock.unlock();
                    }
                    result = true;
                    break;
                }
            }
        } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    }
    return result;
}

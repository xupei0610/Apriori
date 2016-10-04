/**
    Copyright (c) 2016 <PeiXu xuxx0884@umn.edu>

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/**
    File: hashTree.cc
    Purpose: Implementation of Hash Tree used in Apriori algorithm
    @author Pei Xu
    @version 1.0 10/3/2016
*/

#include "hashTree.h"

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

Dataset * HashTree::find(const Data& data, Node *parent_node)
{
    if (data.size() == this->data_size)
    {
        // TODO: Search Legality Check
        // When use this function outside, parameter parent_node should be given
        // null
        // If A possiblly invaild parent is given outside this function, this
        // may cause a failure when hashing

        // Check from the top most node
        if (parent_node == NULL)
        {
            parent_node = this->tree_origin;
        }

        if (parent_node->children.empty() ||
            (data.size() <= parent_node->level))
        {
            return NULL;
        }

        // Get the possible data container.
        // This container may be the exact possible container or just an
        // antecedent node.
        // For a Level n parent, we check its children in Level n+1; so we need
        // to hash the n-th data in the given data (counting from 0)
        int hash_key = hashFunction(data[parent_node->level]);

        if (parent_node->children.find(hash_key) == parent_node->children.end())
        {
            return NULL;
        }

        Node *container = parent_node->children[hash_key];

        // The corresponding child node is not a leaf node
        if (container->dataset.empty())
        {
            return find(data, container);
        }

        for (std::list<Dataset *>::iterator it = container->dataset.begin();
             it != container->dataset.end(); it++)
        {
            if ((*it)->data == data) return *it;
        }
    }

    return NULL;
}

bool HashTree::insert(Dataset *dataset, Node *parent_node)
{
    if (dataset->data.size() == this->data_size)
    {
        // TODO: Insert Legality Check
        // When use this function outside, parameter parent_node should be given
        // null
        // If A possiblly invaild parent is given outside this function, this
        // may cause a failure when hashing

        if (parent_node == NULL)
        {
            parent_node = this->tree_origin;
        }

        int hash_key = hashFunction(*(std::next(dataset->data.begin(),
                                                parent_node->level)));

        if (parent_node->children.find(hash_key) == parent_node->children.end())
        {
            std::list<Dataset *> temp_set;
            temp_set.push_back(dataset);
            parent_node->children[hash_key] = new Node(temp_set,
                                                       parent_node->level + 1);
        }
        else
        {
            Node *container = parent_node->children[hash_key];

            if (container->dataset.empty())
            {
                // Check next level if the corresponding child node is not a
                // leaf, i.e. being splitted.
                return insert(dataset, container);
            }
            else if ((container->dataset.size() == this->max_leafsize) &&
                     (dataset->data.size() > container->level + 1))
            { // The container has been full, and can be splitted.
              // Insert current data to the corresponding child node.

                std::list<Dataset *> temp_set;
                temp_set.push_back(dataset);
                container->children[hash_key] = new Node(temp_set,
                                                         container->level + 1);

                // Insert current node's all dataset into corresponding child
                // nodes respectively

                for (std::list<Dataset *>::iterator it =
                         container->dataset.begin();
                     it != container->dataset.end(); it++)
                {
                    hash_key =
                        hashFunction(*(std::next((*it)->data.begin(),
                                                 container->level)));


                    if (container->children.find(hash_key) ==
                        container->children.end())
                    {
                        temp_set.clear();
                        temp_set.push_back(*it);
                        container->children[hash_key] = new Node(temp_set,
                                                                 container->level +
                                                                 1);
                    }
                    else
                    {
                        container->children[hash_key]->dataset.push_back(*it);
                    }
                }

                // Clear current node's dataset
                container->dataset.clear();
            }
            else
            {
                // Need to split; directly insert to the target container.
                container->dataset.push_back(dataset);
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}

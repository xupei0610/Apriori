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
 File: itemTree.cc
 algorithm
 @author Pei Xu
 @version 0.9 10/24/2016
 */

#include "itemTree.h"

using namespace itemTree;

Node::Node(int data, Node *parent)
{
    this->data    = data;
    this->parent  = parent;
    this->count   = 0;
    this->level   = parent->level + 1;
    this->dataset = parent->dataset;
    this->dataset.push_back(data);
}

Node::Node(int data, Node *parent, std::list<int>trans)
{
    this->data    = data;
    this->parent  = parent;
    this->count   = trans.size();
    this->trans   = trans;
    this->level   = parent->level + 1;
    this->dataset = parent->dataset;
    this->dataset.push_back(data);
}

Node::~Node()
{
    for (auto & c: children) delete c.second;
}

int ItemTree::_min_sup;
int ItemTree::_level;
std::atomic<bool> ItemTree::_boom;
std::mutex ItemTree::_worklock;
std::mutex ItemTree::_worknotelock;
std::queue<Node *> ItemTree::_workbench;
std::list<Node *>  ItemTree::_obsoleted;
std::list<Node *>  ItemTree::_candidates;

ItemTree::ItemTree()
{
    ItemTree::_level   = 0;
    this->_tree_origin = new Node();
    this->_insertlock  = false;
}

ItemTree::~ItemTree()
{
    delete this->_tree_origin;
}

int ItemTree::getCurrentLevel()
{
    return ItemTree::_level;
}

int ItemTree::plant(const int& min_sup,
                    const std::map<int, std::list<int> >& item_table)
{
    if (this->_insertlock == true) return 0;

    int count = 0;
    ItemTree::_min_sup = min_sup;
    this->_insertlock  = true;
    ItemTree::_level   = 1;

    std::vector<std::pair<int, std::list<int> > > vectored_map(
                                                               item_table.begin(), item_table.end());

    std::sort(vectored_map.begin(), vectored_map.end(),
              [](const std::pair<int, std::list<int> >& i,
                 const std::pair<int,  std::list<int> >& j)
              {
                  return i.first < j.first;
              });

    for (auto & it:vectored_map)
    {
        if (it.second.size() >= ItemTree::_min_sup)
        {
            this->_tree_origin->children[it.first] = new Node(it.first,
                                                              this->_tree_origin,
                                                              it.second);
            count++;
        }
    }

    return count;
}

std::list<Node *>ItemTree::grow()
{

    std::list<std::thread> threads;
    int multithread = this->_tree_origin->children.size() >
    MAX_THREADS ?  MAX_THREADS : this->_tree_origin->children.
    size();

    std::queue<Node *>().swap(ItemTree::_workbench);
    ItemTree::_obsoleted.clear();
    ItemTree::_candidates.clear();

    for (auto & c : this->_tree_origin->children)
    {
        ItemTree::_workbench.push(c.second);
    }

    for (int i = 0; i < multithread; i++)
    {
        threads.emplace_back(ItemTree::_sprout, i + 1);
    }
    ItemTree::_boom = true;

    for (auto & th: threads)
    {
        th.join();
    }

    for (auto & d: ItemTree::_obsoleted)
    {
        this->_tree_origin->children.erase(d->data);
        delete d;
    }

    if (!ItemTree::_candidates.empty()) ItemTree::_level++;
    return ItemTree::_candidates;

//     std::list<Node *> result;
//    
//     Node *current;
//     std::queue<Node *> unvisited({ this->_tree_origin });
//     std::list<int>     temp_insect;
//     while (!unvisited.empty()) {
//         current = unvisited.front();
//         unvisited.pop();
//    
//         if (current->level == ItemTree::_level)
//         {
//             for (auto & c: current->parent->children)
//             {
//                 if ((c.first > current->data) &&
//                     (c.second->count >= ItemTree::_min_sup))
//                 {
//                     temp_insect.clear();
//                     std::set_intersection(
//                         c.second->trans.begin(),
//                         c.second->trans.end(),
//                         current->trans.begin(),
//                         current->trans.end(),
//                         std::inserter(temp_insect, temp_insect.begin()));
//    
//                     if (!temp_insect.empty() &&
//                         (temp_insect.size() >= ItemTree::_min_sup))
//                     {
//                         current->children[c.first] = new Node(c.first,
//                                                               current,
//                                                               temp_insect);
//                         // In fact, we have gotten the answer, the size of
//                         // temp_insect is the actual support
//                         result.push_back(current->children[c.first]);
//    
//                         current->children[c.first]->count = 0;
//                     }
//                 }
//             }
//             continue;
//         }
//    
//         for (auto it = current->children.cbegin(); it !=
//     current->children.cend();
//              )
//         {
//             // if (it->second->count < ItemTree::_min_sup)
//             // {
//             //     delete it->second;
//             //     current->children.erase(it++);
//             // }
//             // else
//             // {
//             unvisited.push(it->second);
//             ++it;
//    
//             // }
//         }
//     if (current->children.empty())
//     {
//       current->parent->children.erase(current->data);
//       delete current;
//     }
//     }
//    
//     if (!result.empty()) ItemTree::_level++;
//     return result;
}

void ItemTree::_sprout(const int& tid)
{
    while (!ItemTree::_boom)
    {
        std::this_thread::yield();
    }

    Node *container;
    std::queue<Node *> unvisited;
    std::list<Node *>  result;
    std::list<Node *>  abandoned;
    std::list<int>     temp_insect;
    std::map<int, Node *>::iterator c_end;
    while (true)
    {

        while (!ItemTree::_worklock.try_lock()) {}

        if (ItemTree::_workbench.empty())
        {
            ItemTree::_worklock.unlock();
            break;
        }
        else
        {
            unvisited.push(ItemTree::_workbench.front());
            ItemTree::_workbench.pop();
            ItemTree::_worklock.unlock();
        }
        while (!unvisited.empty())
        {
            container = unvisited.front();
            unvisited.pop();
            if (container->level == ItemTree::_level)
            {
                c_end = container->parent->children.end();
                for (auto c = ++(container->parent->children.find(container->data)); c != c_end; c++)
                {
                    if ((c->first > container->data) &&
                        (c->second->count >= ItemTree::_min_sup))
                    {
//                                                 temp_insect.clear();
//                                                 std::set_intersection(
//                                                     c->second->trans.begin(), c->second->trans.end(),
//                                                     container->trans.begin(),
//                                                     container->trans.end(),
//                                                     std::inserter(temp_insect,
//                                                                   temp_insect.begin()));
                        
//                                                 if (!temp_insect.empty() &&
//                                                     (temp_insect.size() >= ItemTree::_min_sup))
//                                                 {
                        container->children[c->first] = new Node(c->first,
                                                                 container,
                                                                 temp_insect);
                        // In fact, we have gotten the answer in this step, the size of
                        // temp_insect is the actual support
                        result.push_back(container->children[c->first]);

                                                     container->children[c->first]->count = 0;//temp_insect.size();
                                                 }
                    }
                }
                continue;
            }

            for (auto it = container->children.begin();
                 it != container->children.end();                 )
            {
                if (it->second->count < ItemTree::_min_sup)
                {
                    delete it->second;
                    container->children.erase(it++);
                }
                else
                {
                    unvisited.push(it->second);
                    ++it;
                }
            }

            if (container->children.empty())
            {
                if (container->parent->parent == nullptr)
                {
                    abandoned.push_back(container);
                }
                else
                {
                    container->parent->children.erase(container->data);
                    delete container;
                }
            }
        }
    }
    while (!ItemTree::_worknotelock.try_lock()) {}
    for (auto & r: result)
    {
        ItemTree::_candidates.push_back(r);
    }
    for (auto & a: abandoned)
    {
        ItemTree::_obsoleted.push_back(a);
    }
    ItemTree::_worknotelock.unlock();
}

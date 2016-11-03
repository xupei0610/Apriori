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

HashTree::HashTree(const int & hash_range, const int &max_leafsize)
{
    this->_hash_range   = hash_range < 2 ? 1000000 : hash_range;
    this->_max_leafsize = max_leafsize < 1 ? 1 : max_leafsize;
    this->_tree_origin = nullptr;
}

HashTree::~HashTree()
{
    delete this->_tree_origin;
}

Node * HashTree::getRoot()
{
    return this->_tree_origin;
}

int HashTree::getDataSize()
{
    return this->_data_size;
}

void HashTree::build(item::Node *cand, const int & datasize)
{
    this->_last_node_id = 0;
    if (this->_tree_origin != nullptr) delete this->_tree_origin;
    
    this->_tree_origin = new Node();

    if (cand == nullptr) return;
    
    
    this->_data_size = datasize;
    while (cand->right != nullptr)
    {
        cand = cand->right;
        this->_insert(cand);
    }
}

int HashTree::_genNodeID()
{
    this->_last_node_id++;
    return this->_last_node_id;
}

void HashTree::_insert(item::Node * cand)
{
    Node * container = this->_tree_origin;
    int hash_key, key;
    std::map<int, Node *>::iterator ptr;
    
//    if (cand->itemset.size() != this->_data_size) return;
    
    auto it = cand->itemset.begin();
    auto itn = cand->itemset.end();
    for (; it != itn; it++)
    {
        if (container->data_num == 0)
        {
            key = this->hashFunc(*it);
            ptr = container->children.find(key);
            if (ptr == container->children.end())
            {
                container->children[key] = new Node(this->_genNodeID(), container->level+1, cand);
                return;
            }
            else
            {
                container = ptr->second;
            }
        }
        else
        {
            break;
        }
    }
    if (container->data_num == this->_max_leafsize
        && this->_data_size > container->level)
    {
        int new_lvl;
        while(true)
        {
            new_lvl = container->level+1;
            for (auto & da : container->dataset)
            {
                hash_key = this->hashFunc(*(std::next(da->itemset.begin(), container->level)));
                ptr = container->children.find(hash_key);
                if (ptr == container->children.end())
                {
                    container->children[hash_key]= new Node(this->_genNodeID(), new_lvl, da);
                }
                else
                {
                    ptr->second->addData(da);
                }
            }
            container->clearDataset();
            key = this->hashFunc(*it);
            ptr = container->children.find(key);
            if (ptr == container->children.end())
            {
                container->children[key]= new Node(this->_genNodeID(), new_lvl, cand);
                break;
            }
            else
            {
                container = ptr->second;
                if (container->dataset.size() == this->_max_leafsize && this->_data_size > container->level)
                {
                    it++;
                    continue;
                }
                container->addData(cand);
                break;
            }
        }
        return;
    }
    container->addData(cand);

}

std::mutex HashTree::lock;
std::list<trans::Trans *> HashTree::trans_tb;
std::atomic<bool> HashTree::boom;
int HashTree::data_size;
int HashTree::hfrange;

void HashTree::count(std::list<trans::Trans *> *trans_table)
{
    
//    for (auto & tr: *trans_table)
//    {
//        if (tr->col.size() < this->_data_size)
//            continue;
//        this->_count(std::list<int>(), tr->col.begin(), tr, this->_tree_origin);
//    }
    
    std::list<std::thread> threads;
    
    HashTree::trans_tb = *trans_table;
    HashTree::data_size = this->_data_size;
    HashTree::hfrange = this->_hash_range;
    int multithread = trans_table->size() > 60 ? 60 : trans_table->size();
    for (; --multithread > -1;)
    {
        threads.emplace_back(HashTree::_multiCount, this->_tree_origin);
    }
    HashTree::boom = true;
    for (auto & th : threads)
    {
        th.join();
    }
}


void HashTree::_multiCount(Node * tree_origin)
{
    while(!HashTree::boom){std::this_thread::yield();}
    
    trans::Trans * trans;
    
    std::list<std::tuple<std::list<int>, std::list<int>::iterator, Node *> > work;
    std::tuple<std::list<int>, std::list<int>::iterator, Node *> cur_work;
    std::list<int> prefix;
    std::list<int>::iterator key, col_begin, it_begin, col_end;
    Node * node;
    
    std::list<int> diff, max;
    
    while(true)
    {
        while(!HashTree::lock.try_lock()){}
        if (HashTree::trans_tb.empty())
        {
            HashTree::lock.unlock();
            break;
        }
        trans = HashTree::trans_tb.front();
        HashTree::trans_tb.pop_front();
        HashTree::lock.unlock();

        if (trans->col.size() < HashTree::data_size)
            continue;
        col_begin = trans->col.begin();
        col_end = trans->col.end();
        work.push_back(std::make_tuple(std::list<int>(), trans->col.begin(), tree_origin));

        while (!work.empty()) {
            cur_work = work.front();
            work.pop_front();
            std::tie(prefix, key, node) = cur_work;
            
            if (node->data_num > 0)
            {
                
                if (node->level == HashTree::data_size)
//                if (prefix.size() == HashTree::data_size)
                {
                    for (auto & da: node->dataset)
                    {
                        if (da->itemset == prefix)
                        {
                            while(!HashTree::lock.try_lock()){}
                            da->count += trans->weight;
                            HashTree::lock.unlock();
                            break;
                        }
                        if (da->itemset > prefix)
                        {
                            break;
                        }
                    }
                    continue;
                }
                
                max = prefix;
                std::copy(std::next(col_end, prefix.size() - HashTree::data_size), col_end,
                          std::inserter(max, max.end()));
                
                for (auto & da: node->dataset)
                {
                    if (da->itemset > max)
                        break;
                    
                    it_begin = da->itemset.begin();
                    if (std::equal(prefix.begin(), prefix.end(), it_begin))
                    {
                        diff.clear();
                        std::set_difference(it_begin, da->itemset.end(), col_begin, col_end, std::inserter(diff, diff.begin()));
                        if (diff.empty())
                        {
                            while(!HashTree::lock.try_lock()){}
                            da->count += trans->weight;
                            HashTree::lock.unlock();
                        }
                    }
                }
                continue;
            }
            
            auto next_key = std::next(key, 1);
            if (key != std::next(trans->col.end(), node->level - HashTree::data_size))
            {
                work.push_back(std::make_tuple(prefix, next_key, node));
            }
            
            int hash_key = (*key) % HashTree::hfrange;
            prefix.push_back(*key);
            if (node->children.find(hash_key)!=node->children.end())
                work.push_back(std::make_tuple(prefix, next_key, node->children[hash_key]));
        }
    }
}

void HashTree::_count(std::list<int> prefix, const std::list<int>::iterator & key, trans::Trans * trans, Node * node)
{

    if (node->data_num > 0)
    {
        if (node->level == this->_data_size)
        {
            for (auto & da: node->dataset)
            {
                if (da->itemset == prefix)
                {
                    da->count += trans->weight;
                    break;
                }
                if (da->itemset > prefix)
                {
                    break;
                }
            }
            return;
        }
        
        std::list<int> diff;
        std::list<int>::iterator it_begin = trans->col.end();
        std::list<int> max = prefix;
        std::copy(std::next(it_begin, prefix.size() - this->_data_size), it_begin,
                  std::inserter(max, max.end()));
        
        for (auto & da: node->dataset)
        {
            if (da->itemset > max)
            {
                break;
            }
                
            it_begin = da->itemset.begin();
            if (std::equal(prefix.begin(), prefix.end(), it_begin))
            {
                diff.clear();
                std::set_difference(it_begin, da->itemset.end(), trans->col.begin(), trans->col.end(), std::inserter(diff, diff.begin()));
                if (diff.empty())
                {
                    da->count += trans->weight;
                }
            }
        }
        return;
    }
    
    auto next_key = std::next(key, 1);
    if (key != std::next(trans->col.end(), node->level - this->_data_size))
    {
        this->_count(prefix, next_key, trans, node);
    }
    
    int hash_key = this->hashFunc(*key);
    prefix.push_back(*key);
    if (node->children.find(hash_key)!=node->children.end())
        this->_count(prefix, next_key, trans, node->children[hash_key]);
}

int HashTree::hashFunc(const int& v)
{
    return v % this->_hash_range;
}

bool HashTree::find(std::list<int> itemset, const int & count)
{
    if (this->_data_size != itemset.size()) return false;
    
    auto container = this->_tree_origin;
    std::map<int, Node*>::iterator ptr;
    int key;
    for (auto & it : itemset)
    {
        if (container->data_num == 0)
        {
            key = this->hashFunc(it);
            ptr = container->children.find(key);
            if (ptr == container->children.end())
                return false;
            container = container->children[key];
            continue;
        }
    }
    for (auto & da : container->dataset)
    {
        if (da->itemset == itemset)
        {
            while(!HashTree::lock.try_lock()){}
            da->count += count;
            HashTree::lock.unlock();
            return true;
        }
        if (da->itemset > itemset)
        {
            return false;
        }
    }
    return false;
}

bool HashTree::findSubSetCountLast(const std::list<int> & collection, const int & count)
{
//    if (collection.size() == this->_data_size)
//        return this->find(collection, count);
    
    
    bool result = false;
    
    Node *container = this->_tree_origin;
    
    std::queue<std::pair<std::list<int>::iterator, Node *> > unvisited;
    std::pair<std::list<int>::iterator, Node *> current;
    std::list<int> current_data;
    std::list<int>::iterator current_it;
    std::list<int> visited;
    auto dataset = collection;
    auto it_end = std::next(dataset.end(), 1 - (int)(this->_data_size));
    
    int hash_key;
    
    for (auto it = dataset.begin(); it != it_end; it++)
    {
        hash_key = this->hashFunc(*it);
        
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
        if (container->dataset.empty())
        {
            if (this->_data_size - container->level == 1)
            {
                hash_key = this->hashFunc(dataset.back());
                
                if (container->children.find(hash_key) !=
                    container->children.end())
                {
                    container  = container->children[hash_key];
                    current_it = std::next(dataset.end(), -2);
                }
            }
            else
            {
                it_end = std::next(
                                   dataset.end(), 1 + container->level - this->_data_size);
                
                for (auto it = std::next(current_it, 1); it != it_end; it++)
                {
                    hash_key = this->hashFunc(*it);
                    
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
        int cnk = HashTree::calComboNum(dataset.size() - 1, this->_data_size - 1);
        
        int matched = 0;

        if (std::find(visited.begin(), visited.end(),
                      container->id) == visited.end())
        {
            std::list<int> diff;
            visited.push_back(container->id);
            
            for (auto & da : container->dataset)
            {
                if (da->itemset.back() == dataset.back())
                {
                    diff.clear();
                    
                    std::set_difference(da->itemset.begin(), da->itemset.end(),
                                        dataset.begin(),
                                        dataset.end(), std::inserter(diff, diff.begin()));
                    
                    if (diff.empty())
                    {
                        
                        while (!HashTree::lock.try_lock()) {}
                        da->count += count;
                        HashTree::lock.unlock();
                        result = true;
                        matched++;
                                                if (matched>cnk)
                                                {
                        //                            std::cout << "cnk " << cnk << "/n:" << dataset.size()-1 << "/k:" << this->_data_size - 1 << std::endl;
                                                    break;
                                                }
                    }
                }
            }
        }
    }
    
    return result;

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


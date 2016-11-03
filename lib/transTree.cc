#include "transTree.h"

using namespace trans;

std::map<int, std::pair<Node*, Node *> > TransTree::_header_table;

TransTree::TransTree(std::list<Trans *> *trans_table)
{
    this->_tree_origin = new Node();
    this->_clearHeaderTable();
    Node *container;
    std::map<int, Node *>::iterator ptr;
    std::map<int, std::pair<Node *, Node *> >::iterator header_ptr;
    auto tb = *trans_table;
    for (auto & tr: tb)
    {
        container = this->_tree_origin;
        bool w1 = false;
        for (auto & it: tr->collection)
        {
            ptr = container->children.find(it->f_id);
            if (ptr == container->children.end())
            {
                container->children[it->f_id] = new Node(it, container);
                container = container->children[it->f_id];
                header_ptr = TransTree::_header_table.find(it->f_id);
                if (header_ptr == TransTree::_header_table.end())
                {
                    TransTree::_header_table[it->f_id] = std::make_pair(container, new Node(container));
                }
                else
                {
                    header_ptr->second.first->right = container;
                    header_ptr->second.first = container;
                }
            }
            else
            {
                container = ptr->second;
            }
            container->count += tr->weight;
        }
    }
    
}

TransTree::~TransTree()
{
    this->_clearHeaderTable();
    delete this->_tree_origin;
}

void TransTree::_clearHeaderTable()
{
    for (auto & h : TransTree::_header_table)
    {
        delete h.second.second;
    }
    TransTree::_header_table.clear();
}


void TransTree::count(item::ItemTree * item_tree)
{
    auto item_root = item_tree->getRoot();
    TransTree::level = item_tree->getCurrentLevel();
    
    for(auto & c: this->_tree_origin->children)
        if (c.second->length >= TransTree::level)
        _count(item_root, c.second);
    return;
    
    // No efficient performance improvement when using multithreading for this method
    // Basically, time taken by this method is mostly spent on recrusive
    // whose performance will not be improved efficiently when using multithreading
    // since multithreading needs to use container instead of recrusive for the sake of
    // preventing stack overflow, and keeping a container is more expensive than recrusive.
    
//    std::list<std::thread> threads;
//    int multithread = this->_tree_origin->children.size() > 60 ? 60 : this->_tree_origin->children.size();
//    for (;  --multithread > -1;)
//    {
//        threads.emplace_back(TransTree::_multiCount, item_root);
//    }
//    
//    for (auto & c: this->_tree_origin->children)
//    {
//        if (c.second->length >= TransTree::level)
//        TransTree::workbench.push_back(c.second);
//    }
//    TransTree::boom = true;
//    for(auto & th: threads)
//    {
//        th.join();
//    }
}

//void TransTree::_multiCount(item::Node *item_tree)
//{
//    while (TransTree::boom == false) { std::this_thread::yield(); }
//    
//    std::list<std::pair<item::Node *, Node*> > work;
//    std::pair<item::Node *, Node*> curwork;
//    
//    while(true)
//    {
//        while(!TransTree::lock.try_lock()){}
//        if (TransTree::workbench.empty())
//        {
//            
//            TransTree::lock.unlock();
//            break;
//        }
//        work.push_back(std::make_pair(item_tree, workbench.front()));
//        workbench.pop_front();
//        TransTree::lock.unlock();
//        
//        while(!work.empty())
//        {
//            curwork = work.front();
//            work.pop_front();
//
//            if (curwork.first->level == TransTree::level)
//            {
//                while (!TransTree::lock.try_lock()) {}
//                curwork.first->count += curwork.second->count;
//                TransTree::lock.unlock();
//                continue;
//            }
//            
//            auto ptr = curwork.first->children.find(curwork.second->item);
//            if ( ptr != curwork.first->children.end())
//                work.push_back(std::make_pair(ptr->second, curwork.second));
//            
//            if (curwork.second->length > TransTree::level - curwork.first->level)
//            {
//                for(auto & c: curwork.second->children)
//                    work.push_back(std::make_pair(curwork.first, c.second));
//            }
//        }
//    }
//}

void TransTree::_count(item::Node * item_node, Node * cur_node)
{

    if (item_node->level == TransTree::level)
    {
        item_node->count += cur_node->count;
        return;
    }
    
    auto ptr = item_node->children.find(cur_node->item);
    if ( ptr != item_node->children.end())
        _count(ptr->second, cur_node);
    
    if (cur_node->length > TransTree::level - item_node->level)
    {
        for(auto & c: cur_node->children)
            _count(item_node, c.second);
    }
}

std::list<int> TransTree::_passed;
std::stack<int> TransTree::_workbench;
int TransTree::level;
std::mutex TransTree::_lock;
std::list<Node *> TransTree::workbench;
std::atomic<bool> TransTree::_boom;

hashTree::HashTree * TransTree::_hash_tree;

void TransTree::count(hashTree::HashTree * hash_tree)
{
    std::list<std::thread> threads;
    int multithread = TransTree::_passed.size() == 0 ?  60 : TransTree::_passed.size();
    TransTree::_boom = false;
    TransTree::_hash_tree = hash_tree;
    for (int i = 0; i < multithread; i++)
    {
        threads.emplace_back(TransTree::_counter);
    }
    
    std::stack<int>().swap(TransTree::_workbench);
    if (TransTree::_passed.empty())
    {
        for (auto & d : TransTree::_header_table)
        {
            TransTree::_workbench.push(d.first);
        }
    }
    else
    {
        for (auto & d : TransTree::_passed)
        {
            TransTree::_workbench.push(d);
        }
    }
    
    TransTree::_passed.clear();
    
    TransTree::_boom = true;
    for (auto & th: threads)
    {
        th.join();
    }
    
}

void TransTree::_counter()
{
    while (!TransTree::_boom) {std::this_thread::yield();}
    
    Node * container;
    bool success;
    
    while (true)
    {
        while (!TransTree::_lock.try_lock()) {}
        if (TransTree::_workbench.empty())
        {
            TransTree::_lock.unlock();
            break;
        }
        else
        {
            container = TransTree::_header_table[TransTree::_workbench.top()].second;
            TransTree::_workbench.pop();
            TransTree::_lock.unlock();
        }
        
        if (container == nullptr) continue;
        
        success = false;
        while (container->right != nullptr)
        {
            container = container->right;
            if (container->abandoned == true)
            continue;
            
            if (container->level < TransTree::_hash_tree->getDataSize())
            {
                container->abandoned = true;
                continue;
            }
            if (TransTree::_hash_tree->findSubSetCountLast(container->itemset, container->count) == false)
            {
                container->abandoned = true;
                continue;
            }
            success = true;
        }
        
        if (success == true)
        {
            while(!TransTree::_lock.try_lock()){std::this_thread::yield();}
            TransTree::_passed.push_back(container->item);
            TransTree::_lock.unlock();
        }
        
    }
}

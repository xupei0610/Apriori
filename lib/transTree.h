#ifndef TRANSTREE_H
#define  TRANSTREE_H

#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <stack>

#include "trans.h"
#include "hashTree.h"

namespace trans {
    
    struct Node {
        int                  item; // f_id
        int                  count;
        int                 length;
        int                 level;
        Node                *parent;
        std::map<int, Node *>children;
        bool                abandoned;
        std::list<int>      itemset;
        Node * right;
        Node() : item(-1), count(0), length(1), level(0), parent(nullptr), right(nullptr), abandoned(true) {}
        Node(Node *right) : count(0), length(1), level(-1), parent(nullptr), right(right), abandoned(true) {}
        Node(Item *item, Node *parent) : item(item->f_id), count(0), length(1), level(parent->level+1), parent(parent), right(nullptr), abandoned(false)
        {
            this->itemset = parent->itemset;
            this->itemset.push_back(item->f_id);
            this->incLen();
        }
        
        ~Node()
        {
            for (auto & c: children)
                delete c.second;
        }
        
        void incLen()
        {
            if (this->parent != nullptr && this->parent->length == this->length )
            {
                this->parent->length++;
                this->parent->incLen();
            }
        }
        
    };
    
    
    class TransTree {
    public:
        
    private:
        
        Node *_tree_origin;
        int _min_sup;
        static int level;
        static std::list<Node *> workbench;
        static std::mutex _lock;
        static std::list<int> _passed;
        static std::stack<int> _workbench;
        static std::atomic<bool> _boom;
        static std::map<int, std::pair<Node*, Node *> > _header_table;
        static hashTree::HashTree * _hash_tree;
        
    public:
        
        TransTree(std::list<Trans *> * trans_table);
        ~TransTree();
        void count(item::ItemTree * item_tree);
        void count(hashTree::HashTree * hash_tree);

    private:
        void _clearHeaderTable();
        void _count(item::Node * item_node, Node * cur_node);
        static void _counter();
        static void _multiCount(item::Node * item_tree);
        
        
    };
}
#endif // ifndef TRANSTREE_H

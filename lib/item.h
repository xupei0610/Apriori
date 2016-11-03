#ifndef ITEM_H
#define ITEM_H
#include <vector>
#include <list>
#include <map>
#include <mutex>

extern int instances;

namespace item {
    
    const int EMPTY_FID = -1;
    typedef int ItemID;
    struct Item {
        ItemID id;
        int f_id = EMPTY_FID;
        int count = 1;
        int instance = 0;
//        std::list<int> trans;
        Item(const ItemID &id) : id(id){}
        Item(){}
        void inc_instance()
        {
            this->instance++;
            if (this->instance == 1)
                instances++;
        }
        void dec_instance()
        {
            this->instance--;
            if (this->instance == 0)
                instances--;
        }
    };
    
    struct Node {
        int                             item;
        int                             count;
        int                             level;
        Node                           *parent;
        Node                           *left = nullptr;
        Node                           *right=nullptr;
        std::list<int>               itemset;
        std::list<int>                  trans;
        std::map<int, Node *>           children;
        Item *                           item_instance;
        Node() : item(-1), count(0), level(0), parent(nullptr), left(nullptr), right(nullptr), item_instance(nullptr) {}
        
        Node(Node *parent, const int& count, Item * item_instance) : item(item_instance->f_id), count(count),level(parent->level + 1), parent(parent),item_instance(item_instance)
        {
            this->itemset = parent->itemset;
            this->itemset.push_back(item_instance->f_id);
            item_instance->inc_instance();
        }
        Node(Node *parent, const int& count, Item * item_instance, std::list<int> trans) : item(item_instance->f_id), count(count),level(parent->level + 1), parent(parent), trans(trans), item_instance(item_instance)
        {
            this->itemset = parent->itemset;
            this->itemset.push_back(item_instance->f_id);
            item_instance->inc_instance();
        }
        ~Node()
        {
            for (auto & c: children)
            {
                delete c.second;
            }
            
            if (this->left != nullptr)
            {
                this->left->right = this->right;
            }
            
            if (this->right != nullptr)
            {
                this->right->left = std::move(this->left);
            }
            if(this->item_instance!=nullptr) this->item_instance->dec_instance();
        }
    };
    
    class ItemTree {
    public:
        
        ItemTree(const int          & min_sup,
                 std::vector<Item *> *item_table);
        ~ItemTree();
        int   grow();
        int   getCurrentLevel();
        int getMinSup();
        Node* getCurrentItemsets();
        Node * getRoot();
        void useHashTree(const bool & open);
        void compactMode(const bool & open);
        Node * destoryItemNode(Node * trash);
    private:
        
        bool _cheated;
        bool _hash_tree;
        std::vector<Node *> _header_nodes;
        int _level;
        int _min_sup;
        int _grow();
        int _grow22();
//        static void _multiGrow22(const std::map<int, Node *>::iterator it_end, const int & min_sup);
//        int _growCompact();
//        int _grow22Compact();
//        static bool boom;
//        static std::mutex lock;
//        static std::list<std::map<int, Node *>::iterator> workbench;
//        static std::list<std::pair<Node*, Node*> > work_result;
//        static int work_fruit;
        
        std::vector<Item *> * _item_table;
    };
    
}


#endif

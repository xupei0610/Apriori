#ifndef TRANSMANAGER_H
#define TRANSMANAGER_H

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iostream>

#include "item.h"
#include "trans.h"
#include "hashTree.h"
#include "transTree.h"
#include "itemTree.h"

namespace manager {
    
    class Manager {
    public:
        
        Manager();
        ~Manager();
        
        void addTrans(const std::list<item::ItemID>& collection);
        
        std::pair<int, int> ready(const int unsigned & min_sup, const float & min_conf);
        int go();
        void count();
        std::list<item::ItemID> solCode(const std::list<int> & mapped_item_id);
        item::Node * getCurrentItemsets();
        hashTree::Node * getCurrentItemsetsHT();
        void useHashTree(const int unsigned & hash_range, const int unsigned & max_leafsize);
        item::Node * destoryItemNode(item::Node * trash);
        
    private:
        
        std::map<item::ItemID, item::Item>   _uno_item_table;
        std::list<trans::Trans> _uno_trans_table;
        std::vector<item::Item *> item_table;
        std::list<trans::Trans *> trans_table;
        int _min_sup;
        float _min_conf;
        int _level;
        int _cands_num; // This number is used to determine whether a new trans tree should be built
        hashTree::HashTree * _hash_tree;
        trans::TransTree *_trans_tree;
        item::ItemTree *_item_tree;
        
        void _mapItem();
        
        void _arrangeTrans(const int & current_level);
    };
    
}

#endif // ifndef TRANSMANAGER_H

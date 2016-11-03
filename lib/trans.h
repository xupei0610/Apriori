#ifndef TRANS_H
#define TRANS_H

#include <list>

#include "item.h"
#include "itemTree.h"

namespace trans {
    
    
    typedef item::ItemID ItemID;
    typedef item::Item Item;
    
    typedef int TransID;
    
    typedef std::list<int>Itemset;
    
    
    struct Trans {
        int id;
        std::list<Item *>collection;
        std::list<int> col;
        int weight=1;
        int length;
        bool abandoned = false;
    };
    
}


#endif

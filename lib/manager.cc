#include "manager.h"

int instances = 0;

using namespace manager;


Manager::Manager()
{
    this->_trans_tree = nullptr;
    this->_item_tree  = nullptr;
    this->_hash_tree  = nullptr;
}

Manager::~Manager()
{
    if (this->_trans_tree != nullptr) delete this->_trans_tree;
    if (this->_item_tree != nullptr) delete this->_item_tree;
    if (this->_hash_tree != nullptr) delete this->_hash_tree;
}

void Manager::useHashTree(const int unsigned & hash_range, const int unsigned & max_leafsize)
{
    this->_hash_tree = new hashTree::HashTree(hash_range, max_leafsize);
}

int i = 0;
void Manager::addTrans(const std::list<trans::ItemID>& collection)
{
    trans::Trans trans;
    trans.id = i;
    i++;
    std::map<trans::ItemID, trans::Item>::iterator ptr;
    auto uni_col = collection;
    uni_col.sort();
    uni_col.unique();
    for (auto & it: uni_col)
    {
        ptr = this->_uno_item_table.find(it);
        
        if (ptr == this->_uno_item_table.end())
        {
            this->_uno_item_table[it] = item::Item();
            this->_uno_item_table[it].id = it;
            trans.collection.push_back(&(this->_uno_item_table[it]));
        }
        else
        {
            ptr->second.count++;
            trans.collection.push_back(&(ptr->second));
        }
    }
    this->_uno_trans_table.push_back(std::move(trans));
}

std::pair<int, int>  Manager::ready(const int unsigned& min_sup, const float& min_conf)
{
    this->_min_sup  = min_sup;
    this->_min_conf = min_conf;
    std::cout << this->_uno_item_table.size() << std::endl;
    this->_mapItem();
    this->_item_tree  = new item::ItemTree(this->_min_sup, &(this->item_table));
    this->_arrangeTrans(1);
    this->_level = 1;
//    if (this->_hash_tree == nullptr)
        this->_trans_tree  = new trans::TransTree(&(this->trans_table));
//    else
    if (this->_hash_tree != nullptr)
    {
        this->_item_tree->useHashTree(true);
    }
    this->_cands_num = instances;
    return std::make_pair(this->trans_table.size(), this->item_table.size());
}

int Manager::go()
{
    int new_cand = this->_item_tree->grow();
    
    if (this->_hash_tree != nullptr){
        this->_hash_tree->build(this->_item_tree->getCurrentItemsets(), this->_item_tree->getCurrentLevel());
    }
    
//    if (this->_cands_num > 50 && this->_cands_num >  4 * instances / 3)
//    {
////        this->_arrangeTrans(this->_level);
////        if (this->_hash_tree == nullptr)
////        {
//            delete this->_trans_tree;
//            this->_trans_tree  = new trans::TransTree(&(this->trans_table));
////        }
//        this->_cands_num = instances;
//    }
    this->_level++;
    return new_cand;
}

void Manager::count()
{
    if (this->_hash_tree == nullptr)
        this->_trans_tree->count(this->_item_tree);
    else
    {
//        this->_hash_tree->count(&(this->trans_table));
        this->_trans_tree->count(this->_hash_tree);
    }
}

item::Node * Manager::getCurrentItemsets()
{
    return this->_item_tree->getCurrentItemsets();
}

void Manager::_mapItem()
{
    // Ensure to use this just once at the beginning
    if (!this->item_table.empty()) return;
    
    for (auto & it : this->_uno_item_table)
    {
        if (it.second.count >= this->_min_sup)
        {
            this->item_table.push_back(&(it.second));
        }
    }
    
    std::sort(this->item_table.begin(), this->item_table.end(),
              [](const item::Item *i, const item::Item *j) {
                  return i->count > j->count;
              });
    
    for (int i = this->item_table.size(); --i > -1;)
    {
        this->item_table[i]->f_id = i;
    }
}

void Manager::_arrangeTrans(const int& current_level)
{
    auto it = std::begin(this->_uno_trans_table);
    this->trans_table.clear();
    
    // Remove Infrequent Item
    std::list<item::Item *>::iterator resize_it, itn, itn_end;
    
    while (it != std::end(this->_uno_trans_table))
    {
        if (it->abandoned == false)
        {
            resize_it = std::remove_if(
                                       it->collection.begin(), it->collection.end(),
                                       [](const item::Item *i) {
                                           return i->instance < 1;
                                       });
            it->length = std::distance(it->collection.begin(), resize_it);
            if (it->length > current_level)
            {
                it->collection.resize(it->length);
                it->collection.sort([](const item::Item *i, const item::Item *j) {
                    return i->f_id < j->f_id;
                });
                this->trans_table.push_back(&(*it));
                if (this->_hash_tree!=nullptr)
                {
                    it->col.clear();
                    itn_end = it->collection.end();
                    for (itn = it->collection.begin(); itn != itn_end; itn++)
                    {
                        it->col.push_back((*itn)->f_id);
                    }
//                    if (current_level == 1)
//                    {
//                        for(auto & item: it->collection)
//                            item->trans.push_back(it->id);
//                    }
                }
                ++it;
                continue;
            }
        }
        it = this->_uno_trans_table.erase(it);
    }
    
    if (this->trans_table.size() > 1)
    {
        // Sort collection for each trans
        this->trans_table.sort(
                               [](const trans::Trans *i, const trans::Trans *j) {
                                   return i->length > j->length;
                               });
        // remove repeated trans
        // Now use trans tree
           auto itt = std::begin(this->trans_table);
           std::list<trans::Trans *>::iterator next;
           std::list<item::Item *>::iterator itn, itn_end, itnn;
           int i = 0;
           bool diff;
           while (itt != --std::end(this->trans_table))
           {
             next = std::next(itt, 1);
             if ((*itt)->length == (*next)->length )
             {
               diff = false;
               itn = std::begin((*itt)->collection);
               itn_end = std::end((*itt)->collection);
               itnn = std::begin((*next)->collection);
               for (;itn!=itn_end; itn++, itnn++)
               {
                 if ((*itn)->f_id != (*itnn)->f_id)
                 {
                   diff = true;
                   break;
                 }
               }
               if (diff == false)
               {
                 (*next)->abandoned = true;
                 (*itt)->weight += (*next)->weight;
                 this->trans_table.erase(next);
                 continue;
               }
             }
             itt++;
             i++;
           }
           this->trans_table.resize(i);
    }
}

item::Node * Manager::destoryItemNode(item::Node *trash)
{
    return this->_item_tree->destoryItemNode(trash);
}

std::list<item::ItemID> Manager::solCode(const std::list<int> & mapped_item_id)
{
    std::list<item::ItemID> result;
    for (auto & fid : mapped_item_id)
    {
        result.push_back(this->item_table[fid]->id);
    }
    return result;
}


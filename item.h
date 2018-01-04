#ifndef _ITEM_H_LOADED
#define _ITEM_H_LOADED

typedef struct
{
    unsigned int itemId;
    unsigned int quantity;
} Item_t;

typedef struct
{
    unsigned int itemId;
    unsigned int rank;
    char *name;
} ItemName_t;

typedef struct
{
    ItemName_t *storage;
    ItemName_t **index;
    unsigned int count;
} ItemNameList_t;

int ItemLoad(ItemNameList_t *namelist, const char *itemFolder);
unsigned int ItemGetID(const char *itemname, const ItemNameList_t *namelist);
int ItemFromString(const char *str, Item_t *item, const ItemNameList_t *namelist);
const char *ItemName(unsigned int itemId, const ItemNameList_t *namelist);
ItemName_t *ItemNameObj(unsigned int itemId, ItemNameList_t *namelist);

#endif

#ifndef _ITEMS_H_LOADED
#define _ITEMS_H_LOADED

#include "item.h"

typedef struct
{
    Item_t *storage;
    unsigned int count;
    unsigned int capacity;
} Items_t;

Items_t *ItemsCreate(void);
void ItemsDestory(Items_t *items);
Items_t *ItemsDuplicate(Items_t *items);

int ItemsAdd(Items_t *items, const Item_t *item);
int ItemsRemove(Items_t *items, const Item_t *item);
int ItemsCheck(const Items_t *items, const Item_t *item);
int ItemsAdd_S(Items_t *items, unsigned int itemID, unsigned int quantity);
int ItemsRemove_S(Items_t *items, unsigned int itemID, unsigned int quantity);
int ItemsCheck_S(const Items_t *items, unsigned int itemID, unsigned int quantity);

const Item_t *ItemsFromIndex(const Items_t *items, unsigned int index);
const Item_t *ItemsFromItemId(const Items_t *items, unsigned int itemID);

#endif

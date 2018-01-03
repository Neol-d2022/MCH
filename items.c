#include "items.h"

#include <stdlib.h>
#include <string.h>

#define _ITEMS_INITIAL_CAPACITY 8
#define _ITEMS_ABORT_ON_NO_MEMORY

static int _ItemsSorting_by_itemID(const void *a, const void *b);
static Item_t *_ItemsSearch(const Item_t *key, const Items_t *items, int (*sortingFunc)(const void *a, const void *b));
static int _ItemsCreateItem(Items_t *items, const Item_t *item, int (*sortingFunc)(const void *a, const void *b));
static int _ItemsExpandCapacity(Items_t *items);
static int _ItemsDestroyItem(Items_t *items, const Item_t *itemFromStorage);
static int _ItemsCheckItem(const Items_t *items, const Item_t *item, Item_t **out);

Items_t *ItemsCreate(void)
{
    Items_t *items;

    items = (Items_t *)malloc(sizeof(*items));
    if (items)
    {
        memset(items, 0, sizeof(*items));
        items->storage = (Item_t *)malloc(sizeof(*(items->storage)) * _ITEMS_INITIAL_CAPACITY);
        if (items->storage == NULL)
        {
            free(items);
            items = NULL;
        }
        else
            items->capacity = _ITEMS_INITIAL_CAPACITY;
    }

#ifdef _ITEMS_ABORT_ON_NO_MEMORY
    if (items == NULL)
        abort();
#endif

    return items;
}

void ItemsDestory(Items_t *items)
{
    free(items->storage);
    free(items);
}

Items_t *ItemsDuplicate(Items_t *items)
{
    Items_t *itemsDup;

    itemsDup = (Items_t *)malloc(sizeof(*itemsDup));

    if (itemsDup)
    {
        itemsDup->storage = (Item_t *)malloc(sizeof(*(itemsDup->storage)) * items->capacity);
        if (itemsDup->storage == NULL)
        {
            free(itemsDup);
            itemsDup = NULL;
        }
        else
        {
            memcpy(itemsDup->storage, items->storage, sizeof(*(itemsDup->storage)) * items->count);
            itemsDup->capacity = items->capacity;
            itemsDup->count = items->count;
        }
    }

#ifdef _ITEMS_ABORT_ON_NO_MEMORY
    if (itemsDup == NULL)
        abort();
#endif

    return itemsDup;
}

int ItemsAdd(Items_t *items, const Item_t *item)
{
    Item_t *existing = _ItemsSearch(item, items, _ItemsSorting_by_itemID);

    if (existing)
    {
        existing->quantity += item->quantity;
        return 0; // Non-existing
    }
    else if (_ItemsCreateItem(items, item, _ItemsSorting_by_itemID) == 0)
        return 1; // Existing
    else
        return 2; // No Memory
}

int ItemsRemove(Items_t *items, const Item_t *item)
{
    Item_t *existing;
    int result;

    result = _ItemsCheckItem(items, item, &existing);
    switch (result)
    {
    case 0:
        existing->quantity -= item->quantity;
        break;
    case 3:
        result = _ItemsDestroyItem(items, existing);
        result = 0;
        break;
    }

    return result;
}

int ItemsCheck(const Items_t *items, const Item_t *item)
{
    Item_t *existing;
    int result;

    result = _ItemsCheckItem(items, item, &existing);
    switch (result)
    {
    case 0:
    case 3:
        return 0;
    default:
        return result;
    }
}

int ItemsAdd_S(Items_t *items, unsigned int itemID, unsigned int quantity)
{
    Item_t item;

    item.itemId = itemID;
    item.quantity = quantity;

    return ItemsAdd(items, &item);
}

int ItemsRemove_S(Items_t *items, unsigned int itemID, unsigned int quantity)
{
    Item_t item;

    item.itemId = itemID;
    item.quantity = quantity;

    return ItemsRemove(items, &item);
}

int ItemsCheck_S(const Items_t *items, unsigned int itemID, unsigned int quantity)
{
    Item_t item;

    item.itemId = itemID;
    item.quantity = quantity;

    return ItemsCheck(items, &item);
}

const Item_t *ItemsFromIndex(const Items_t *items, unsigned int index)
{
    if (index >= items->count)
        return NULL;
    else
        return (items->storage) + index;
}

const Item_t *ItemsFromItemId(const Items_t *items, unsigned int itemID)
{
    Item_t item;
    Item_t *existing;

    item.itemId = itemID;
    existing = _ItemsSearch(&item, items, _ItemsSorting_by_itemID);

    return existing;
}

// ================================
// =                              =
// ================================

static int _ItemsSorting_by_itemID(const void *a, const void *b)
{
    Item_t *c = (Item_t *)a;
    Item_t *d = (Item_t *)b;

    if (c->itemId > d->itemId)
        return 1;
    else if (c->itemId < d->itemId)
        return -1;
    else
        return 0;
}

static Item_t *_ItemsSearch(const Item_t *key, const Items_t *items, int (*sortingFunc)(const void *a, const void *b))
{
    if (items->count)
        return (Item_t *)bsearch(key, items->storage, items->count, sizeof(*key), sortingFunc);
    else
        return NULL;
}

static int _ItemsCreateItem(Items_t *items, const Item_t *item, int (*sortingFunc)(const void *a, const void *b))
{
    if (items->count == items->capacity)
        if (_ItemsExpandCapacity(items))
            return 1;

    memcpy(items->storage + ((items->count)++), item, sizeof(*item));
    qsort(items->storage, items->count, sizeof(*(items->storage)), sortingFunc);
    return 0;
}

static int _ItemsExpandCapacity(Items_t *items)
{
    Item_t *storage;

    storage = realloc(items->storage, sizeof(*(items->storage)) * (items->capacity << 1));

    if (storage == NULL)
    {
#ifdef _ITEMS_ABORT_ON_NO_MEMORY
        abort();
#else
        return 1;
#endif
    }

    items->storage = storage;
    return 0;
}

static int _ItemsDestroyItem(Items_t *items, const Item_t *itemFromStorage)
{
    size_t index;
    unsigned int remaining;

    index = ((size_t)itemFromStorage - (size_t)(items->storage)) / sizeof(*itemFromStorage);
    if (index >= items->count)
        abort();

    remaining = items->count - index - 1;
    if (remaining)
        memmove(items->storage + index, items->storage + index + 1, sizeof(*(items->storage)) * remaining);
    items->count -= 1;

    return 0;
}

static int _ItemsCheckItem(const Items_t *items, const Item_t *item, Item_t **out)
{
    Item_t *existing = _ItemsSearch(item, items, _ItemsSorting_by_itemID);

    if (existing)
    {
        if (out)
            *out = (Item_t *)existing;
        if (existing->quantity < item->quantity)
            return 2; // Not Enough
        else if (existing->quantity == item->quantity)
            return 3; // Exact count
        else
            return 0; // OK
    }

    return 1; // Not Found
}

#include "item.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _ITEM_ABORT_ON_NO_MEMORY

typedef struct _ItemLoad_struct_t
{
    ItemName_t item;
    struct _ItemLoad_struct_t *next;
} _ItemLoad_t;

static void _ItemLoad_CleanUp(_ItemLoad_t *head);
static int _ItemLoad_AddItem(_ItemLoad_t ***current, const char *filename, unsigned int *autoincrease);
static int _ItemLoad_ToArray(_ItemLoad_t *head, ItemName_t **out, ItemName_t ***indexOut, size_t itemscount);
static int _ItemLoad_Sorting_by_name(const void *a, const void *b);
static void _ItemLoad_AddItem_TrimString(char *str);
static int _ItemLoad_Sorting_by_itemId(const void *a, const void *b);

int ItemLoad(ItemNameList_t *namelist, const char *itemFolder)
{
    struct stat s;
    DIR *dir;
    struct dirent *dp;
    char *fullpath;
    size_t length, strlength, itemscount;
    _ItemLoad_t *head = NULL;
    _ItemLoad_t **current = &head;
    unsigned int autoincrease = 0;

    dir = opendir(itemFolder);
    if (dir == NULL)
    {
        fprintf(stderr, "[ItemLoad] Cannot read DIR '%s'.\n", itemFolder);
        return 1;
    }

    itemscount = 0;
    while ((dp = readdir(dir)))
    {
        strlength = strlen(itemFolder);
        length = strlength + strlen(dp->d_name) + 1;
        fullpath = (char *)malloc(length);
        if (fullpath == NULL)
        {
            fprintf(stderr, "[ItemLoad] Out of memory.\n");
#ifdef _ITEM_ABORT_ON_NO_MEMORY
            abort();
#else
            _ItemLoad_CleanUp(head);
            closedir(dir);
            return 1;
#endif
        }
        memcpy(fullpath, itemFolder, strlength + 1);
        strcat(fullpath, dp->d_name);
        if (stat(fullpath, &s) == 0)
        {
            if (S_ISREG(s.st_mode))
            {
                if (_ItemLoad_AddItem(&current, fullpath, &autoincrease) != 0)
                {
                    _ItemLoad_CleanUp(head);
                    free(fullpath);
                    closedir(dir);
                    return 1;
                }
                itemscount += 1;
            }
        }
        free(fullpath);
    }

    closedir(dir);
    namelist->count = itemscount;
    return _ItemLoad_ToArray(head, &(namelist->storage), &(namelist->index), itemscount);
}

unsigned int ItemGetID(const char *itemname, const ItemNameList_t *namelist)
{
    ItemName_t key, *result;

    key.name = (char *)itemname;
    result = (ItemName_t *)bsearch(&key, namelist->storage, namelist->count, sizeof(*(namelist->storage)), _ItemLoad_Sorting_by_name);

    if (result == NULL)
        return 0;
    else
        return result->itemId;
}

int ItemFromString(const char *str, Item_t *item, const ItemNameList_t *namelist)
{
    const char *q, *v;
    unsigned int quantity, itemId;

    q = str;
    v = strchr(q, ',');
    if (v == NULL)
        return 1;
    v += 1;

    if (sscanf(q, "%u,", &quantity) != 1)
        return 1;

    itemId = ItemGetID(v, namelist);
    if (v == 0)
        return 1;

    item->quantity = quantity;
    item->itemId = itemId;
    return 0;
}

const char *ItemName(unsigned int itemId, const ItemNameList_t *namelist)
{
    ItemName_t _key, *key, **result;

    _key.itemId = itemId;
    key = &_key;
    result = (ItemName_t **)bsearch(&key, namelist->index, namelist->count, sizeof(*(namelist->index)), _ItemLoad_Sorting_by_itemId);
    if (result == NULL)
        return NULL;
    else
        return (*result)->name;
}

ItemName_t *ItemNameObj(unsigned int itemId, ItemNameList_t *namelist)
{
    ItemName_t _key, *key, **result;

    _key.itemId = itemId;
    key = &_key;
    result = (ItemName_t **)bsearch(&key, namelist->index, namelist->count, sizeof(*(namelist->index)), _ItemLoad_Sorting_by_itemId);
    return *result;
}

// ================================
// =                              =
// ================================

static void _ItemLoad_CleanUp(_ItemLoad_t *head)
{
    _ItemLoad_t *current, *next;

    current = head;
    while (current != NULL)
    {
        next = current->next;
        free(current->item.name);
        free(current);
        current = next;
    }
}

static int _ItemLoad_AddItem(_ItemLoad_t ***current, const char *filename, unsigned int *autoincrease)
{
    char buf[1024];
    FILE *f;
    _ItemLoad_t *n;

    f = fopen(filename, "r");
    if (f == NULL)
        return 1;

    n = (_ItemLoad_t *)malloc(sizeof(*n));
    if (n == NULL)
#ifdef _ITEM_ABORT_ON_NO_MEMORY
        abort();
#else
    {
        fclose(f);
        return 1;
    }
#endif

    if (fgets(buf, sizeof(buf), f) == NULL)
    {
        fclose(f);
        free(n);
        return 1;
    }
    fclose(f);

    _ItemLoad_AddItem_TrimString(buf);
    n->item.name = strdup(buf);
    if (n->item.name == NULL)
    {
        free(n);
        return 1;
    }
    n->item.itemId = ++(*autoincrease);
    n->item.rank = 0;
    n->next = **current;
    (**current) = n;
    *current = &(n->next);

    fprintf(stderr, "[_ItemLoad_AddItem] '%s' (%u) has just added to ItemList.\n", n->item.name, n->item.itemId);
    return 0;
}

static int _ItemLoad_ToArray(_ItemLoad_t *head, ItemName_t **out, ItemName_t ***indexOut, size_t itemscount)
{
    _ItemLoad_t *current, *next;
    ItemName_t *itemname, **index;

    itemname = (ItemName_t *)malloc(sizeof(*itemname) * itemscount);
    if (itemname == NULL)
#ifdef _ITEM_ABORT_ON_NO_MEMORY
        abort();
#else
        return 1;
#endif

    index = (ItemName_t **)malloc(sizeof(*itemname) * itemscount);
    if (index == NULL)
#ifdef _ITEM_ABORT_ON_NO_MEMORY
        abort();
#else
        free(itemname);
    return 1;
#endif

    itemscount = 0;
    current = head;
    while (current != NULL)
    {
        next = current->next;
        memcpy(itemname + itemscount, &(current->item), sizeof(*itemname));
        free(current);
        current = next;
        index[itemscount] = itemname + itemscount;
        itemscount += 1;
    }

    qsort(itemname, itemscount, sizeof(*itemname), _ItemLoad_Sorting_by_name);
    qsort(index, itemscount, sizeof(*index), _ItemLoad_Sorting_by_itemId);
    *out = itemname;
    *indexOut = index;
    fprintf(stderr, "[_ItemLoad_ToArray] Total loaded items: %u.\n", (unsigned int)itemscount);
    return 0;
}

static int _ItemLoad_Sorting_by_name(const void *a, const void *b)
{
    ItemName_t *c = (ItemName_t *)a;
    ItemName_t *d = (ItemName_t *)b;

    return strcmp(c->name, d->name);
}

static void _ItemLoad_AddItem_TrimString(char *str)
{
    char *p;

    p = strchr(str, '\n');
    if (p)
        *p = '\0';
    p = strchr(str, '\r');
    if (p)
        *p = '\0';
}

static int _ItemLoad_Sorting_by_itemId(const void *a, const void *b)
{
    ItemName_t *c = *(ItemName_t **)a;
    ItemName_t *d = *(ItemName_t **)b;

    if (c->itemId > d->itemId)
        return 1;
    else if (c->itemId < d->itemId)
        return -1;
    else
        return 0;
}

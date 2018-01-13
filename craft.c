#include "craft.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define _CRAFT_ABORT_ON_NO_MEMORY

typedef struct _CraftLoadRecipeList_struct_t
{
    Recipe_t item;
    struct _CraftLoadRecipeList_struct_t *next;
} _CraftLoadRecipeList_t;

static void _CraftLoadRecipeList_CleanUp(_CraftLoadRecipeList_t *head);
static int _CraftLoadRecipeList_AddItem(_CraftLoadRecipeList_t ***current, const ItemNameList_t *itemnamelist, const char *filename);
static int _CraftLoadRecipeList_ToArray(_CraftLoadRecipeList_t *head, RecipeList_t *recipesList, size_t itemscount);
static int _CraftLoadRecipeList_ToArray_Sorting_by_outputItemID(const void *a, const void *b);
static void _CraftLoadRecipeList_AddItem_TrimString(char *str);
static int _CraftSortRecipe_Sorting_by_recipeOutputItemRank(const void *a, const void *b);

int CraftLoadRecipeList(RecipeList_t *recipesList, const ItemNameList_t *itemnamelist, const char *recipeFolder)
{
    struct stat s;
    DIR *dir;
    struct dirent *dp;
    char *fullpath;
    char *filetype;
    size_t length, strlength, itemscount;
    _CraftLoadRecipeList_t *head = NULL;
    _CraftLoadRecipeList_t **current = &head;

    dir = opendir(recipeFolder);
    if (dir == NULL)
    {
        fprintf(stderr, "[CraftLoadRecipeList] Cannot read DIR '%s'.\n", recipeFolder);
        return 1;
    }

    itemscount = 0;
    while ((dp = readdir(dir)))
    {
        strlength = strlen(recipeFolder);
        length = strlength + strlen(dp->d_name) + 1;
        fullpath = (char *)malloc(length);
        if (fullpath == NULL)
        {
            fprintf(stderr, "[CraftLoadRecipeList] Out of memory.\n");
#ifdef _CRAFT_ABORT_ON_NO_MEMORY
            abort();
#else
            _CraftLoadRecipeList_CleanUp(head);
            closedir(dir);
            return 1;
#endif
        }
        memcpy(fullpath, recipeFolder, strlength + 1);
        strcat(fullpath, dp->d_name);
        filetype = strstr(dp->d_name, ".txt");
        if (filetype != NULL)
        {
            if (strlen(filetype) == strlen(".txt"))
            {
                if (stat(fullpath, &s) == 0)
                {
                    if (S_ISREG(s.st_mode))
                    {
                        if (_CraftLoadRecipeList_AddItem(&current, itemnamelist, fullpath) != 0)
                        {
                            _CraftLoadRecipeList_CleanUp(head);
                            free(fullpath);
                            closedir(dir);
                            return 1;
                        }
                        itemscount += 1;
                    }
                }
            }
        }
        free(fullpath);
    }

    closedir(dir);
    return _CraftLoadRecipeList_ToArray(head, recipesList, itemscount);
}

int Craft(const Item_t *targetItem, Items_t *itemRequired, Items_t *itemRemaining, const RecipeList_t *recipesList, const ItemNameList_t *itemnamelist, CraftStep_t ***cs)
{
    Recipe_t key;
    Item_t item, diff;
    Recipe_t *result;
    const char *targetitemname = ItemName(targetItem->itemId, itemnamelist), *ingredientname;
    CraftStep_t *ncs;
    unsigned int numberOfIngredients, i, multipler;
    int returned;

    fprintf(stderr, "[Craft] Crafting '%s' x%u.\n", targetitemname, targetItem->quantity);
    key.output = *targetItem;
    result = (Recipe_t *)bsearch(&key, recipesList->storage, recipesList->count, sizeof(*(recipesList->storage)), _CraftLoadRecipeList_ToArray_Sorting_by_outputItemID);
    if (result == NULL)
    {
        fprintf(stderr, "[Craft] '%s' is base ingredient.\n", targetitemname);
        ItemsAdd(itemRequired, targetItem);
        ItemsAdd(itemRemaining, targetItem);
        return 0;
    }
    else
    {
        numberOfIngredients = result->input->count;
        multipler = (targetItem->quantity + (result->output.quantity - 1)) / result->output.quantity;
        fprintf(stderr, "[Craft] '%s' x%u requires recipe '%s(x%u)' to be done %u times.\n", targetitemname, targetItem->quantity, targetitemname, result->output.quantity, multipler);

        for (i = 0; i < numberOfIngredients; i += 1)
        {
            item = *ItemsFromIndex(result->input, i);
            item.quantity *= multipler;
            ingredientname = ItemName(item.itemId, itemnamelist);
            fprintf(stderr, "[Craft] Ingredient '%s' x%u are required for item '%s' x%u.\n", ingredientname, item.quantity, targetitemname, targetItem->quantity);
            if (ItemsCheck(itemRemaining, &item) != 0)
            {
                diff.itemId = item.itemId;
                diff.quantity = 0;
                if (ItemsCheck(itemRemaining, &diff) == 0)
                    diff.quantity = item.quantity - ItemsFromItemId(itemRemaining, diff.itemId)->quantity;
                else
                    diff.quantity = item.quantity;

                fprintf(stderr, "[Craft] Ingredient '%s' x%u are not available for item '%s' x%u.\n", ingredientname, diff.quantity, targetitemname, targetItem->quantity);
                returned = Craft(&diff, itemRequired, itemRemaining, recipesList, itemnamelist, cs);
                if (returned != 0)
                    return returned;
            }

            fprintf(stderr, "[Craft] Ingredient '%s' x%u has just been prepared for item '%s' x%u.\n", ingredientname, item.quantity, targetitemname, targetItem->quantity);
            if (ItemsRemove(itemRemaining, &item) != 0)
                abort();
        }

        item = result->output;
        item.quantity *= multipler;
        ItemsAdd(itemRemaining, &item);
        fprintf(stderr, "[Craft] '%s' x%u has just been crafted.\n", targetitemname, item.quantity);
        if (cs)
        {
            ncs = (CraftStep_t *)malloc(sizeof(*ncs));
            if (ncs == NULL)
            {
#ifdef _CRAFT_ABORT_ON_NO_MEMORY
                abort();
#else
                return 1;
#endif
            }
            ncs->multipler = multipler;
            ncs->r = result;
            ncs->next = **cs;
            **cs = ncs;
            *cs = &(ncs->next);
        }
        return 0;
    }
}

int RecipePrint(const Recipe_t *r, const ItemNameList_t *itemnamelist, unsigned int multipler)
{
    unsigned int i, n;
    int result = 0;

    n = 9;
    result += fprintf(stdout, "\nOUTPUT: '%s(x%u)'(x%u)\n", ItemName(r->output.itemId, itemnamelist), r->output.quantity, multipler);
    result += fprintf(stdout, "CRAFT MODE: %u\n", r->craftMode);
    for (i = 0; i < n; i += 1)
    {
        if ((r->inputSlots)[i] != 0)
        {
            result += fprintf(stdout, "INPUT: %2u is '%s'(x%u)\n", i, ItemName((r->inputSlots)[i], itemnamelist), (r->inputSlots)[i + n]);
        }
    }

    return result;
}

unsigned int ItemRank(unsigned int itemId, ItemNameList_t *namelist, const RecipeList_t *recipesList)
{
    Recipe_t key;
    ItemName_t *obj;
    Recipe_t *result;
    unsigned int maxRank, i, n, rank;

    obj = ItemNameObj(itemId, namelist);
    if (obj == NULL)
        return (unsigned int)-1;

    if (obj->rank != 0)
        return obj->rank; // Cached
    else
    {
        key.output.itemId = itemId;
        key.output.quantity = 0;
        result = (Recipe_t *)bsearch(&key, recipesList->storage, recipesList->count, sizeof(*(recipesList->storage)), _CraftLoadRecipeList_ToArray_Sorting_by_outputItemID);
        if (result == NULL)
            return (obj->rank = 1); // Base
        else
        {
            maxRank = 0;
            n = result->input->count;
            for (i = 0; i < n; i += 1)
            {
                rank = ItemRank((result->input->storage)[i].itemId, namelist, recipesList);
                if (rank > maxRank)
                    maxRank = rank;
            }
            return (obj->rank = maxRank + 1);
        }
    }
}

void CraftSortRecipe(CraftStep_t *head, CraftStep_t **array, unsigned int *length, ItemNameList_t *namelist, RecipeList_t *recipesList)
{
    void *args[] = {namelist, recipesList};
    const struct Recipe_struct_t *r;
    CraftStep_t *cur, *next, *a;
    size_t n, m, i;

    n = 0;
    cur = head;
    while (cur)
    {
        next = cur->next;
        n += 1;
        cur = next;
    }

    a = (CraftStep_t *)malloc(sizeof(*a) * n);
    if (a == NULL)
#ifdef _CRAFT_ABORT_ON_NO_MEMORY
        abort();
#else
        return;
#endif

    i = 0;
    cur = head;
    while (cur)
    {
        next = cur->next;
        memcpy(a + i, cur, sizeof(*a));
        a[i].next = (CraftStep_t *)args;
        i += 1;
        free(cur);
        cur = next;
    }

    qsort(a, n, sizeof(*a), _CraftSortRecipe_Sorting_by_recipeOutputItemRank);
    m = n;
    r = NULL;
    for (i = 0; i < n; i += 1)
    {
        if ((size_t)r == (size_t)a[i].r)
        {
            m -= 1;
            a[i].multipler += a[i - 1].multipler;
            a[i - 1].r = NULL;
        }
        else
            r = a[i].r;
    }

    qsort(a, n, sizeof(*a), _CraftSortRecipe_Sorting_by_recipeOutputItemRank);
    *array = a;
    *length = m;
}

// ================================
// =                              =
// ================================

static void _CraftLoadRecipeList_CleanUp(_CraftLoadRecipeList_t *head)
{
    _CraftLoadRecipeList_t *current, *next;

    current = head;
    while (current != NULL)
    {
        next = current->next;
        ItemsDestory(current->item.input);
        free(current->item.inputSlots);
        free(current);
        current = next;
    }
}

static int _CraftLoadRecipeList_AddItem(_CraftLoadRecipeList_t ***current, const ItemNameList_t *itemnamelist, const char *filename)
{
    char buf[1024];
    unsigned int slots[32];
    Item_t item;
    FILE *f;
    _CraftLoadRecipeList_t *n;
    const char *s;
    unsigned int slot, i, numberOfIngredients;

    f = fopen(filename, "r");
    if (f == NULL)
        return 1;

    n = (_CraftLoadRecipeList_t *)malloc(sizeof(*n));
    if (n == NULL)
#ifdef _ITEM_ABORT_ON_NO_MEMORY
        abort();
#else
    {
        fclose(f);
        return 1;
    }
#endif
    n->item.input = ItemsCreate();
    if (n->item.input == NULL)
    {
        fclose(f);
        free(n);
        return 1;
    }

    if (fgets(buf, sizeof(buf), f) == NULL)
    {
        fprintf(stderr, "[_CraftLoadRecipeList_AddItem] Cannot read '%s'.\n", filename);
        ItemsDestory(n->item.input);
        fclose(f);
        free(n);
        return 1;
    }
    _CraftLoadRecipeList_AddItem_TrimString(buf);
    if (ItemFromString(buf, &item, itemnamelist))
    {
        fprintf(stderr, "[_CraftLoadRecipeList_AddItem] '%s' (as recipe output) is not a item.\n", buf);
        ItemsDestory(n->item.input);
        fclose(f);
        free(n);
        return 1;
    }
    n->item.output.itemId = item.itemId;
    n->item.output.quantity = item.quantity;

    memset(slots, 0, sizeof(slots));
    while (fgets(buf, sizeof(buf), f) != NULL)
    {
        _CraftLoadRecipeList_AddItem_TrimString(buf);
        if (strlen(buf) == 0)
            break;

        s = strchr(buf, ',');
        if (s == NULL)
        {
            fprintf(stderr, "[_CraftLoadRecipeList_AddItem] '%s' (as recipe input) does not contain valid slot number(0 ~ 8).\n", buf);
            ItemsDestory(n->item.input);
            fclose(f);
            free(n);
            return 1;
        }
        if (sscanf(buf, "%u,", &slot) != 1)
        {
            fprintf(stderr, "[_CraftLoadRecipeList_AddItem] '%s' (as recipe input) does not contain valid slot number(0 ~ 8).\n", buf);
            ItemsDestory(n->item.input);
            fclose(f);
            free(n);
            return 1;
        }
        if (slot >= 9)
        {
            fprintf(stderr, "[_CraftLoadRecipeList_AddItem] '%s' (as recipe input) does not contain valid slot number(0 ~ 8).\n", buf);
            ItemsDestory(n->item.input);
            fclose(f);
            free(n);
            return 1;
        }
        if (ItemFromString(s + 1, &item, itemnamelist))
        {
            fprintf(stderr, "[_CraftLoadRecipeList_AddItem] '%s' (as recipe input) does not contain valid input ingredient.\n", buf);
            ItemsDestory(n->item.input);
            fclose(f);
            free(n);
            return 1;
        }
        ItemsAdd(n->item.input, &item);
        slots[slot] = item.itemId;
        slots[slot + 9] = item.quantity;
    }
    fclose(f);

    n->item.inputSlots = (unsigned int *)malloc(sizeof(*(n->item.inputSlots)) * 18);
    if (n->item.inputSlots == NULL)
    {
        ItemsDestory(n->item.input);
        free(n);
        return 1;
    }
    memcpy(n->item.inputSlots, &slots, sizeof(*(n->item.inputSlots)) * 18);
    n->item.craftMode = 0;
    n->next = **current;
    **current = n;
    *current = &(n->next);

    numberOfIngredients = n->item.input->count;
    fprintf(stderr, "[_CraftLoadRecipeList_AddItem] Recipe '%s' has just been loaded.\n", ItemName(n->item.output.itemId, itemnamelist));
    for (i = 0; i < numberOfIngredients; i += 1)
        fprintf(stderr, "\t'%s' (%u) x%u.\n", ItemName(ItemsFromIndex(n->item.input, i)->itemId, itemnamelist), ItemsFromIndex(n->item.input, i)->itemId, ItemsFromIndex(n->item.input, i)->quantity);

    return 0;
}

static int _CraftLoadRecipeList_ToArray(_CraftLoadRecipeList_t *head, RecipeList_t *recipesList, size_t itemscount)
{
    _CraftLoadRecipeList_t *current, *next;
    Recipe_t *recipes;

    recipes = (Recipe_t *)malloc(sizeof(*recipes) * itemscount);
    if (recipes == NULL)
#ifdef _CRAFT_ABORT_ON_NO_MEMORY
        abort();
#else
        return 1;
#endif

    itemscount = 0;
    current = head;
    while (current != NULL)
    {
        next = current->next;
        memcpy(recipes + itemscount, &(current->item), sizeof(*recipes));
        free(current);
        current = next;
        itemscount += 1;
    }

    recipesList->storage = recipes;
    recipesList->count = itemscount;
    qsort(recipes, itemscount, sizeof(*recipes), _CraftLoadRecipeList_ToArray_Sorting_by_outputItemID);
    fprintf(stderr, "[_CraftLoadRecipeList_ToArray] Total loaded recipes: %u.\n", (unsigned int)itemscount);
    return 0;
}

static int _CraftLoadRecipeList_ToArray_Sorting_by_outputItemID(const void *a, const void *b)
{
    Recipe_t *c = (Recipe_t *)a;
    Recipe_t *d = (Recipe_t *)b;

    if (c->output.itemId > d->output.itemId)
        return 1;
    else if (c->output.itemId < d->output.itemId)
        return -1;
    else
        return 0;
}

static void _CraftLoadRecipeList_AddItem_TrimString(char *str)
{
    char *p;

    p = strchr(str, '\n');
    if (p)
        *p = '\0';
    p = strchr(str, '\r');
    if (p)
        *p = '\0';
}

static int _CraftSortRecipe_Sorting_by_recipeOutputItemRank(const void *a, const void *b)
{
    void **args;
    CraftStep_t *c = (CraftStep_t *)a;
    CraftStep_t *d = (CraftStep_t *)b;
    ItemNameList_t *namelist;
    RecipeList_t *recipesList;
    unsigned int e, f;

    if (c->r == NULL && d->r == NULL)
        return 0;
    else if (c->r == NULL)
        return 1;
    else if (d->r == NULL)
        return -1;
    else
    {
        args = (void **)c->next;
        namelist = (ItemNameList_t *)args[0];
        recipesList = (RecipeList_t *)args[1];
        e = ItemRank(c->r->output.itemId, namelist, recipesList);
        f = ItemRank(d->r->output.itemId, namelist, recipesList);
        if (e > f)
            return 1;
        else if (e < f)
            return -1;
        else
        {
            e = c->r->output.itemId;
            f = d->r->output.itemId;
            if (e > f)
                return 1;
            else if (e < f)
                return -1;
            else
                return 0;
        }
    }
}

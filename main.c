#include "craft.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void _TrimString(char *str);

int main(void)
{
    char buf[1024];
    ItemNameList_t itemnamelist;
    RecipeList_t recipelist;
    Item_t item;
    Items_t *required, *remaining, *items2make;
    CraftStep_t *head = NULL, **current, *next, *cur;
    unsigned int numberOfItems, i;

    if (ItemLoad(&itemnamelist, "itemnames/"))
        return 1;

    if (CraftLoadRecipeList(&recipelist, &itemnamelist, "recipes/"))
        return 1;

    items2make = ItemsCreate();
    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        _TrimString(buf);
        if (strlen(buf) == 0)
            break;
        if (ItemFromString(buf, &item, &itemnamelist) == 0)
            ItemsAdd(items2make, &item);
    }

    required = ItemsCreate();
    remaining = ItemsCreate();
    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        _TrimString(buf);
        if (strlen(buf) == 0)
            break;
        if (ItemFromString(buf, &item, &itemnamelist) == 0)
            ItemsAdd(remaining, &item);
    }

    current = &head;
    numberOfItems = items2make->count;
    for (i = 0; i < numberOfItems; i += 1)
        Craft(ItemsFromIndex(items2make, i), required, remaining, &recipelist, &itemnamelist, &current);
    ItemsDestory(items2make);

    numberOfItems = required->count;
    fprintf(stdout, "[Required] Item Type: %u\n", numberOfItems);
    for (i = 0; i < numberOfItems; i += 1)
    {
        item = *ItemsFromIndex(required, i);
        fprintf(stdout, "\t%s x%u\n", ItemName(item.itemId, &itemnamelist), item.quantity);
    }
    ItemsDestory(required);

    numberOfItems = remaining->count;
    fprintf(stdout, "[Remaining] Item Type: %u\n", numberOfItems);
    for (i = 0; i < numberOfItems; i += 1)
    {
        item = *ItemsFromIndex(remaining, i);
        fprintf(stdout, "\t%s x%u\n", ItemName(item.itemId, &itemnamelist), item.quantity);
    }
    ItemsDestory(remaining);

    cur = head;
    while (cur != NULL)
    {
        next = cur->next;
        RecipePrint(cur->r, &itemnamelist, cur->multipler);
        free(cur);
        cur = next;
    }

    return 0;
}

static void _TrimString(char *str)
{
    char *p;

    p = strchr(str, '\n');
    if (p)
        *p = '\0';
    p = strchr(str, '\r');
    if (p)
        *p = '\0';
}

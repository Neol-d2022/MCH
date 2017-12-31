#ifndef _CRAFT_H_LOADED
#define _CRAFT_H_LOADED

#include "items.h"

typedef struct
{
    Item_t output;
    Items_t *input;
    unsigned int *inputSlots;
    unsigned int craftMode;
} Recipe_t;

typedef struct
{
    Recipe_t *storage;
    unsigned int count;
} RecipeList_t;

int CraftLoadRecipeList(RecipeList_t *recipesList, const ItemNameList_t *itemnamelist, const char *recipeFolder);
int Craft(const Item_t *targetItem, Items_t *itemRequired, Items_t *itemRemaining, const RecipeList_t *recipesList, const ItemNameList_t *itemnamelist);

#endif

#ifndef _CRAFT_H_LOADED
#define _CRAFT_H_LOADED

#include "craftstep.h"
#include "items.h"

typedef struct Recipe_struct_t
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
int Craft(const Item_t *targetItem, Items_t *itemRequired, Items_t *itemRemaining, const RecipeList_t *recipesList, const ItemNameList_t *itemnamelist, CraftStep_t ***cs);
int RecipePrint(const Recipe_t *r, const ItemNameList_t *itemnamelist, unsigned int multipler);
unsigned int ItemRank(unsigned int itemId, ItemNameList_t *namelist, const RecipeList_t *recipesList);
void CraftSortRecipe(CraftStep_t *head, CraftStep_t **array, unsigned int *length, ItemNameList_t *namelist, RecipeList_t *recipesList);

#endif

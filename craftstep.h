#ifndef _CRAFTING_STEP_H_LOADED
#define _CRAFTING_STEP_H_LOADED

typedef struct CraftStep_struct_t
{
    const struct Recipe_struct_t *r;
    struct CraftStep_struct_t *next;
    unsigned int multipler;
} CraftStep_t;

#endif

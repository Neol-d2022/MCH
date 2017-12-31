# MCH
MCH - Minecraft Crafting Helper

## To Download The Source Code
```shell
git clone https://github.com/Neol-d2022/MCH.git
```

## To Compile
```shell
cd MCH/
make
```

## To test
```shell
make test < test_input.txt
```

## To Use
- Edit files under `itemnames/` and `recipes/` as you wish.
- ```
  ./minecraft_crafting_helper.exe
  ...
  ```
- *Example*
  - *INPUT*
  ```
  ./minecraft_crafting_helper.exe
  1,木鎬
  1,木斧
  <EOF>
  ```
  - *OUTPUT*
  ```
  [Required] Item Type: 1
  	木材 x8
  [Remaining] Item Type: 2
  	木斧 x1
  	木鎬 x1
  [_ItemLoad_AddItem] '木斧' (1) has just added to ItemList.
  [_ItemLoad_AddItem] '木鎬' (2) has just added to ItemList.
  [_ItemLoad_AddItem] '木棒' (3) has just added to ItemList.
  [_ItemLoad_AddItem] '木材' (4) has just added to ItemList.
  [_ItemLoad_ToArray] Total loaded items: 4.
  [_CraftLoadRecipeList_AddItem] Recipe '木斧' has just loaded.
  	'木棒' (3) x2.
  	'木材' (4) x3.
  [_CraftLoadRecipeList_AddItem] Recipe '木鎬' has just loaded.
  	'木棒' (3) x2.
  	'木材' (4) x3.
  [_CraftLoadRecipeList_AddItem] Recipe '木棒' has just loaded.
  	'木材' (4) x2.
  [_CraftLoadRecipeList_ToArray] Total loaded recipes: 3.
  [Craft] Crafting '木斧' x1.
  [Craft] '木斧' x1 requires recipe '木斧' to be done 1 times.
  [Craft] Ingredien '木棒' x2 are required for item '木斧' x1.
  [Craft] Ingredien '木棒' x2 are not available for item '木斧' x1.
  [Craft] Crafting '木棒' x2.
  [Craft] '木棒' x2 requires recipe '木棒' to be done 1 times.
  [Craft] Ingredien '木材' x2 are required for item '木棒' x2.
  [Craft] Ingredien '木材' x2 are not available for item '木棒' x2.
  [Craft] Crafting '木材' x2.
  [Craft] '木材' is base ingredient.
  [Craft] Ingredien '木材' x2 has just been prepared for item '木棒' x2.
  [Craft] '木棒' x4 has just been crafted.
  [Craft] Ingredien '木棒' x2 has just been prepared for item '木斧' x1.
  [Craft] Ingredien '木材' x3 are required for item '木斧' x1.
  [Craft] Ingredien '木材' x3 are not available for item '木斧' x1.
  [Craft] Crafting '木材' x3.
  [Craft] '木材' is base ingredient.
  [Craft] Ingredien '木材' x3 has just been prepared for item '木斧' x1.
  [Craft] '木斧' x1 has just been crafted.
  [Craft] Crafting '木鎬' x1.
  [Craft] '木鎬' x1 requires recipe '木鎬' to be done 1 times.
  [Craft] Ingredien '木棒' x2 are required for item '木鎬' x1.
  [Craft] Ingredien '木棒' x2 has just been prepared for item '木鎬' x1.
  [Craft] Ingredien '木材' x3 are required for item '木鎬' x1.
  [Craft] Ingredien '木材' x3 are not available for item '木鎬' x1.
  [Craft] Crafting '木材' x3.
  [Craft] '木材' is base ingredient.
  [Craft] Ingredien '木材' x3 has just been prepared for item '木鎬' x1.
  [Craft] '木鎬' x1 has just been crafted.
  ```
  - It basically prints all ingredients you need to make specified items, and what are left after crafting.
    - In this case, the program is told to print ingredients need to craft one `木鎬`(wooden pickaxe) and one `木斧`(wooden axe).
    - At the end, the program said that we need eight `木材`(wood planks) to craft those two items. The debug messages also suggests that `木棒`(sticks) were crafted during the process.

## Notes about `itemnames/` and `recipes/`
- All files under these two directories are except subfolders are read when the program starts.
  - Filename is not important, since all files will be enumerate. The content of it matters.
  - **File Format Under `itemnames/`**
    - ```
      <item name>
      ```
    - Yup, that's simple.
  - **File Format Under `recipes/`**
    - ```
      <output count>,<output item name>
      <slot number>,<ingredient #1 count>,<ingredient #1 name>
      <slot number>,<ingredient #2 count>,<ingredient #2 name>
      ...
      ```
    - If you are still confused, see `recipes/axe.txt`. I am pretty sure you are smart enough to understand that, as you are reading this.
  - **To add new item/recipe**, just create a new file under these two folders, don't care about the file name, and that the program to the rest.
  - The files under subfolders under `itemnames/` and `recipes/` are ignored.

## To Be Done
- Print recipes used.
- Step-by-step crafting process.
- A GUI maybe.
- Translation? If I have so many times.

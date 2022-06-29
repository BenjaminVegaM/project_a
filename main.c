#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "list.h"
#include "treemap.h"

typedef struct
{
    int hp; //Health Points
    int atk; //Potential Damage
    int spd; //Speed, the fastest will act first
    int def; //Reduces Physical Damage Received
    int res; //Reduces Magical Damage Received
} Stats;

typedef struct
{
    int id; //Identification number of the item
    char name[41]; //Name of the Item (also used as Key)
    char description[140]; //Description of the Item
    int type; //0 Text, 1 Health, 2 Stats Buff, 3 Stats Debuff
    int subType; //Depends on the main type
    int uses; //Amount of times it can be used
    int value; //Amount of healing/damage/stats/etc it gives
} Item;

typedef struct
{
    char name[41]; //Name of the Weapon
    char description[140]; //Description of the Item
    int rarity; //Rarity of the Weapon
    int durability; //Amount of times left it can be used
    int effectiveness; //If it's the same as the oponent's race, will deal more damage
    int damageType; //1 Physical, 2 Magical, 0 True Damage
    Stats * stats; //Increase to the Character's Stats
} Weapon;

typedef struct
{
    int isPlayer; //1 Player, 0 otherwise
    char name[51]; //Name of the Character
    int race; //A number that represents the race
    int level; //Current Level of the character
    int experience; //Current Experience of the Character
    int expToLvlUp; //Experience necesary to level up
    int state; //State of the character in combat (0 Normal, 1 Attacking, 2 Defending, 3 Stunned, 4 Sleeping)
    int currentHP; //Current Health Points of the Character
    Stats * stats; //Stats of the Character
    Weapon * weapons[4]; //Inventory of Weapons of the Character
    int currentWeapon; //Current Selected Weapon of the Character
    TreeMap * items; //Inventory of Items of the Character
} Character;

typedef struct 
{
    List * list; //The List of Weapons Available
    int amount; //The amount of weapons available
} WeaponsListNode;

typedef struct 
{
    List * list; //The List of Items Available
    int amount; //The amount of items available
} ItemsListNode;

typedef struct
{
    FILE * file; //The File containning the Races available
    int amount; //The amount of races available
}RacesFile;

typedef struct
{
    int state; //Current state of the game
    int floor; //Current Floor of the Run
    int room; //Current Room of the Floor
    int turn; //Current Turn of the current Battle
    int score; //Score of the Run
}RunManager;

//Global Variables
ItemsListNode * itemsList;
WeaponsListNode * weaponsList;
RacesFile * racesFile;
FILE * highScoresFile;

//Turns a String to lowercase
char * stringToLower(char * string)
{
    int i;
    for(i = 0 ; string[i] ; i++) string[i] = tolower(string[i]);
    return string;
}

//generates a random Integer number within a range
int randIntLimits(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

//generates a random Float number within a range
float randFloatLimits(int lower, int upper)
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

//Función para tomar un string de una coordenada específica de un archivo .csv
const char *getCSVField (char * tmp, int k)
{

    int open_mark = 0, ini_i=0, i=0, j=0;
    char* ret=(char*) malloc (201*sizeof(char));
    while(tmp[i]!='\0' && tmp[i]!='\n'){

        if(tmp[i]== '\"'){
            open_mark = 1-open_mark;
            if(open_mark) ini_i = i+1;
            i++;
            continue;
        }

        if(open_mark || tmp[i]!= ','){
            if(k==j) ret[i-ini_i] = tmp[i];
            i++;
            continue;
        }

        if(tmp[i]== ','){
            if(k==j) {
               ret[i-ini_i] = 0;
               return ret;
            }
            j++; ini_i = i+1;
        }

        i++;
    }

    if(k==j) {
       ret[i-ini_i] = 0;
       return ret;
    }


    return NULL;
}

//For TreeMap
int lowerThanString(void* key1, void* key2)
{
    char * k1 = (char*) key1;
    char * k2 = (char*) key2;
    if(k1 < k2) return 1;
    return 0;
}
int lowerThanInt(void* key1, void* key2)
{
    int * k1 = (int*) key1;
    int * k2 = (int*) key2;
    if(*k1 < *k2) return 1;
    return 0;
}

//Funcion que requiere presionar enter para proseguir, sirve para generar una pausa.
void pressEnterToContinue()
{
    printf("...");
    getchar();
}

//Frees memory of a character
void freeCharacter(Character * chara)
{
    for(int w = 0 ; w < 4 ; w++) if(chara->weapons[w] != NULL) free(chara->weapons[w]);
    Pair * pair = firstTreeMap(chara->items);
    while(pair != NULL)
    {
        Item * item = pair->value;
        free(item);
        free(pair);
        pair = nextTreeMap(chara->items);
    }
    free(chara);
}

//----------SAVE----------SAVE----------SAVE----------SAVE----------SAVE----------SAVE----------

//Saves the items of the character in the file
void saveItems(Character * chara, FILE * f)
{
    Pair * pair = firstTreeMap(chara->items);
    Item * item;
    if(pair == NULL) fprintf(f,"NULL");
    else
    {
        do
        {
            item = pair->value;
            printf("%s\n", item->name);
            fprintf(f,"%i,\"%s\",\"%s\",%i,%i,%i,%i,%i\n",item->id, item->name, item->description, item->type, item->subType, item->uses, item->value);
            pair = nextTreeMap(chara->items);
        }while(pair != NULL);
    }
}

//Saves the info of the character in the file
void saveCharacter(Character * chara, FILE * saveFile)
{
    fprintf(saveFile, "\"%s\",%i,%i,%i,%i,%i,%i,%i,", chara->name, chara->race, chara->level, chara->experience, chara->expToLvlUp, chara->state, chara->currentHP, chara->currentWeapon);
    fprintf(saveFile, "%i,%i,%i,%i,%i\n", chara->stats->hp, chara->stats->atk, chara->stats->spd, chara->stats->def, chara->stats->res);
    
    for(int i = 1 ; i < 4 ; i++)
    {
        if(chara->weapons[i] == NULL) fprintf(saveFile,"NULL\n");
        else fprintf(saveFile, "\"%s\",\"%s\",%i,%i,%i,%i,%i,%i,%i,%i,%i\n", chara->weapons[i]->name, chara->weapons[i]->description, chara->weapons[i]->rarity, chara->weapons[i]->durability, chara->weapons[i]->effectiveness, chara->weapons[i]->damageType, chara->weapons[i]->stats->hp, chara->weapons[i]->stats->atk, chara->weapons[i]->stats->spd, chara->weapons[i]->stats->def, chara->weapons[i]->stats->res);
    }
}

//Saves the data of the current run in the save files
int saveRun(Character * playerChara, Character * enemyChara, RunManager * runManager)
{
    printf("Saving the game...\n");
    FILE * saveFile = fopen("./save/save.csv", "w");
    if(saveFile == NULL)
    {
        printf("There was an error opening the file [save.csv], please check the game files before trying again.\n(If necesary please delete the file, a new one will be created)\n");
        return 1;
    }
    int i;
    printf("Saving Numbers\n");
    fprintf(saveFile, "%i,%i,%i,%i\n", runManager->floor, runManager->room, runManager->turn, runManager->score);

    printf("Saving Player Character\n");
    saveCharacter(playerChara, saveFile);

    printf("Saving Current Enemy Character\n");
    saveCharacter(enemyChara, saveFile);
    fclose(saveFile);
    
    printf("Saving Player Character ITEMS\n");
    FILE * playerItemsFile = fopen("./save/playerItems.csv", "w");
    saveItems(playerChara, playerItemsFile);
    fclose(playerItemsFile);

    printf("Saving Current Enemy Character ITEMS\n");
    FILE * enemyItemsFile = fopen("./save/enemyItems.csv", "w");
    saveItems(enemyChara, playerItemsFile);
    fclose(enemyItemsFile);

    freeCharacter(playerChara);
    freeCharacter(enemyChara);

    printf("Game Saved!\n");
    return 0;
}

// Generates random stats based on the Level of the character
Stats * genRanCharaStats(int level)
{
    float randNum;
    Stats * stats = (Stats *) malloc (sizeof (Stats));

    randNum = randFloatLimits(80, 120)/100;
    stats->hp = level * randNum + 5;

    randNum = fmax(1, randFloatLimits(80, 120)/100);
    stats->atk = level * randNum;

    randNum = randFloatLimits(80, 120)/100;
    stats->spd = level * randNum;

    randNum = randFloatLimits(80, 120)/100;
    stats->def = level * randNum;

    randNum = randFloatLimits(80, 120)/100;
    stats->res = level * randNum;

    return stats;
}

const char * getRaceString(int nInt)
{
    char l[1024];
    fseek(racesFile->file, 0, SEEK_SET);
    if(fgets(l, 1023, racesFile->file) == NULL || getCSVField(l, nInt-1) == NULL) return "Unkown";
    return getCSVField(l, nInt-1);
}

//----------WEAPONS----------WEAPONS----------WEAPONS----------WEAPONS----------WEAPONS----------

const char * getEffectivenessString(int nInt)
{
    if(nInt == 0) return "None";
    return getRaceString(nInt);
}

const char * getRarityString(int nInt)
{
    switch(nInt)
    {
        case 1: return "Common";
        case 2: return "Uncommon";
        case 3: return "Rare";
        case 4: return "Epic";
        case 5: return "Legendary";
        case 6: return "Mythic";
    }
}

const char * getDamageTypeString(int nInt)
{
    switch(nInt)
    {
        case 1: return "Physical";
        case 2: return "Magical";
    }
}

//Shows the info of a weapon
void printWeapon(Weapon * weapon)
{
    printf("Name: [%s]\n", weapon->name);
    printf("Description: [%s]\n", weapon->description);
    printf("Rarity: [%s]\n", getRarityString(weapon->rarity));
    printf("Durability: [%i]\n", weapon->durability);
    printf("Effectiveness: [%s]\n", getEffectivenessString(weapon->effectiveness));
    printf("Damage Type: [%s]\n", getDamageTypeString(weapon->damageType));
    if(weapon->stats->hp < 0) printf("Self-Damage: [%i]\n", weapon->stats->hp*-1);
    if(weapon->stats->hp > 0) printf("Self-Healing: [%i]\n", weapon->stats->hp);
    printf("Stats:\n");
    printf("  Atk [%i]\n", weapon->stats->atk);
    printf("  Spd [%i]\n", weapon->stats->spd);
    printf("  Def [%i]\n", weapon->stats->def);
    printf("  Res [%i]\n", weapon->stats->res);
}

void showWeapons(Character * chara)
{
    if(chara->weapons[1] == NULL && chara->weapons[2] == NULL && chara->weapons[3] == NULL)
    {
        printf("It's completely empty!\n");
        return;
    }

    for(int i = 1 ; i < 4 ; i++)
    {
        printf("---Slot %i---\n", i);
        if(chara->weapons[i] != NULL) printWeapon(chara->weapons[i]);
        else printf("Empty\n");
    }
}

//Reserves memory for a Weapon and initializes the variables
Weapon * createWeapon(char * name, char * description, int rarity, int durability, int effectiveness, int damageType, int hp, int atk, int spd, int def, int res)
{
    Weapon * weapon = (Weapon*) malloc (sizeof(Weapon));
    strcpy(weapon->name, name);
    strcpy(weapon->description, description);
    weapon->rarity = rarity;
    weapon->durability = durability;
    weapon->effectiveness = effectiveness;
    weapon->damageType = damageType;
    weapon->stats = (Stats*) malloc (sizeof(Stats));
    weapon->stats->hp = hp;
    weapon->stats->atk = atk;
    weapon->stats->spd = spd;
    weapon->stats->def = def;
    weapon->stats->res = res;
    return weapon;
}

int getRanWpnRarity(int lvl)
{
    int r;
    //dependiendo del nivel tiene mas o menos chance de cada rareza
         if(lvl > 10) r = randIntLimits(3,5);
    else if(lvl > 7) r = randIntLimits(2,4);
    else if(lvl > 5) r = randIntLimits(2,3);
    else if(lvl > 3) r = randIntLimits(1,3);
    else r = randIntLimits(1,2);

    return r;
}

//Chekcs if there is any weapon of the rarity received, if there is at least one then returns a random one of that rarity
Weapon * getRanWeapon(int rarity)
{
    if(weaponsList[rarity-1].amount < 1)
    {
        printf("There are no weapons of rarity [%s] on [weapons.csv], please add at least one.", getRarityString(rarity));
        exit(1);
    }
    Weapon * weapon = firstList(weaponsList[rarity-1].list);
    if(weapon == NULL)
    {
        printf("There are no weapons of rarity [%s] on [weapons.csv], please add at least one.", getRarityString(rarity));
        exit(1);
    }
    int i = randIntLimits(0,weaponsList[rarity-1].amount-1); //should be a randInt from 0 to the max amount of weapons of rarity
    for(int j = 0 ; j < i ; j++)
    {
        weapon = nextList(weaponsList[rarity-1].list);
        if(weapon == NULL)
        {
            printf("The weapon n° %i doesn't exist, going to the last one\n", j);
            weapon = prevList(weaponsList[rarity-1].list);
            break;
        }
    }
    weapon = createWeapon(weapon->name, weapon->description, weapon->rarity, weapon->durability, weapon->effectiveness, weapon->damageType, weapon->stats->hp, weapon->stats->atk, weapon->stats->spd, weapon->stats->def, weapon->stats->res);
    return weapon;
}

//Gives a weapon to a character, if they are full then asks to throw one
void giveWeapon(Character * chara, Weapon * weapon)
{
    int b = 1;
    while(b < 4 && chara->weapons[b] != NULL) b++;
    if(b < 4) chara->weapons[b] = weapon;
    else
    {
        if(chara->isPlayer == 1)
        {
            int w;
            getchar();
            printf("You don't have more space for weapons.\n");
            printf("Choose one to throw away.\n");
            for(w = 1 ; w < 4 ; w++) printf("%i. %s\n", w, chara->weapons[w]->name);
            printf("4. (New)%s", weapon->name);
            do
            {
                scanf("%i", &w);
            }while(w < 1 || w > 4);
            if(w == 4)
            {
                printf("You left behind the %s.\nMaybe it wasn't that good.\n", weapon->name);
                free(weapon);   
            }
            else
            {
                printf("You threw the %s.\nJust don't regret it later.\n", chara->weapons[w]->name);
                free(chara->weapons[w]);
                chara->weapons[w] = weapon;
            }
        }
    }
}

//Changes the character's current weapon to the next available one, starting backwards to leave the empty hands as the last option.
void chooseNextAvailableWeapon(Character * chara)
{
    int i = 3;
    while(chara->weapons[i] == NULL) i--;
    chara->currentWeapon = i;
}

//----------ITEMS----------ITEMS----------ITEMS----------ITEMS----------ITEMS----------ITEMS----------

//Returns a string based on the type received
const char * getTypeString(int type, int subType)
{
    switch(type)
    {
        case 1:
        {
            if(subType == 1) return "Healing";
            if(subType == 2) return "Damaging";
        }
        case 2: return "Stats Buff";
        case 3: return "Stats Debuff";
        default: return "Useless";
    }
}

//checks if the inventory of items is empty
int checkIfEmptyItems(Character * chara)
{
    Pair * pair = firstTreeMap(chara->items);
    if(pair == NULL)
    {
        printf("It's completely empty!\n");
        return 0;
    }
    else return 1;
}

//Shows the info of an item
void printItem(Item * item)
{
    printf("ID: [%i]\n", item->id);
    printf("Name: [%s]\n", item->name);
    printf("Description: [%s]\n", item->description);
    printf("Type: [%s]\n", getTypeString(item->type, item->subType));
    printf("Uses Left: [%i]\n", item->uses);
    if(item->type != 0)
    {
        printf("%s: [%i", getTypeString(item->type, item->subType), item->value);
        if(item->type == 1 && item->subType == 1) printf("% HP +5");
        printf("]\n");
    }

}

//Shows all items on a character's inventory
void showItems(Character * chara)
{
    if(checkIfEmptyItems(chara) == 0) return;
    Item * item;
    Pair * pair = firstTreeMap(chara->items);
    int i = 0;
    do
    {
        i++;
        item = pair->value;
        printf("---Item %i---\n", i);
        printItem(item);
        pair = nextTreeMap(chara->items);
    }while(pair != NULL);
}

void useItemUseless(Character * chara1, Character * chara2, Item * item)
{
    if(chara1->isPlayer == 1)
    {
        printf("You used the %s", item->name);
        switch(item->value)
        {
            case 0: printf(". It suddenly dissapeared"); break;
            case 1: printf(", but it did nothing."); break;
            case 2: printf(". %s started laughing for a second.", chara2->name); break;
            case 3: printf(" but it broke the moment you touched it."); break;
            case 4: printf(". You look great now!"); break;
            default: printf(". Nothing happened."); break;
        }
        printf("\n");
    }
    else printf("The enemy used an item but it did nothing.\n");
}

void useItemHealth(Character * chara1, Character * chara2, Item * item)
{
    int amount;
    if(item->subType == 1)
    {
        amount = 5 + ((chara1->stats->hp * item->value)/100);
        if(chara1->isPlayer == 1) printf("You used the %s!\nYou have restored %i health.\n", item->name, amount);
        else printf("The enemy used a Healing Item!\nThey have restored some of their health.\n");
        chara1->currentHP = fmin(chara1->stats->hp, (chara1->currentHP + amount));
    }
    else
    {
        if(chara1->isPlayer == 1) printf("You used the %s!\n", item->name);
        else printf("The enemy used a Damaging Item!\n");
        int damageDealt = fmax(1, (item->value - chara2->stats->res));
        if(chara2->state == 2)
        {
            damageDealt = 0;
            printf("%s dodged the attack!!\n", chara2->name);
            chara2->state = 0;
        }
        else
        {
            printf("It dealt %i damage!\n", damageDealt);
            chara2->currentHP = fmax(1, (chara2->currentHP - damageDealt));
            if(chara2->currentHP == 1) printf("%s barely survived the attack!\n", chara2->name);
        }
    }
}

void useItemStats(Character * chara1, Character * chara2, Item * item)
{
    if(item->value >= 0)
    {
        if(chara1->isPlayer == 1) printf("You used the %s!\nYour stats have increased!\n", item->name);
        else printf("The enemy used a Buff Item!\nTheir stats have increased!\n");
        switch(item->subType)
        {
            case 1: chara1->stats->atk+=item->value; break;
            case 2: chara1->stats->spd+=item->value; break;
            case 3: chara1->stats->def+=item->value; break;
            case 4: chara1->stats->res+=item->value; break;
            default: chara1->stats->atk+=item->value; break;
        }
    }
    else
    {
        if(chara1->isPlayer == 1) printf("You used the %s!\nThe enemy stats have decreased!\n", item->name);
        else printf("The enemy used a Debuff Item!\nYour stats have decreased!\n");
        switch(item->subType)
        {
            case 1: chara2->stats->atk = fmax(0, chara2->stats->atk - item->value); break;
            case 2: chara2->stats->spd = fmax(0, chara2->stats->spd - item->value); break;
            case 3: chara2->stats->def = fmax(0, chara2->stats->def - item->value); break;
            case 4: chara2->stats->res = fmax(0, chara2->stats->res - item->value); break;
            default: chara2->stats->atk = fmax(0, chara2->stats->atk - item->value); break;
        }
    }
}

void useAiItem(Character * chara1, Character * chara2)
{
    Pair * pair = firstTreeMap(chara1->items);
    Item * item = pair->value;
    if(item->type == 1) useItemHealth(chara1, chara2, item);
    else if(item->type > 1 && item->type < 8) useItemStats(chara1, chara2, item);
    else useItemUseless(chara1, chara2, item);
    item->uses--;
    if(item->uses == 0) eraseTreeMap(chara1->items, &item->id);
}

int usePlayerItem(Character * chara1, Character * chara2, int id)
{
    Pair * pair = searchTreeMap(chara1->items, &id);
    if(pair == NULL)
    {
        printf("You don't have that item!\n");
        getchar();
        return 0;
    }
    printf("You are using the item of ID %i\n", id);
    getchar();
    Item * item = pair->value;
    if(item->type == 1) useItemHealth(chara1, chara2, item);
    else if(item->type > 1 && item->type < 8) useItemStats(chara1, chara2, item);
    else useItemUseless(chara1, chara2, item);
    item->uses--;
    if(item->uses == 0) eraseTreeMap(chara1->items, &item->id);
    return 1;
}

//Gives an item to a character or increases its uses left
void giveItem(Character * chara, Item * item)
{
    Pair * auxPair = searchTreeMap(chara->items, &item->id);
    if(auxPair != NULL)
    {
        Item * auxItem = auxPair->value;
        auxItem->uses+=item->uses;
    }
    else
    {
        insertTreeMap(chara->items, &item->id, item);
    }
    printf("Successfully given a %s\n", item->name);    
}

//Reserves memory for an Item and initializes the variables
Item * createItem(int id, char * name, char * description, int type, int subType, int uses, int value)
{
    Item * item = (Item*) malloc (sizeof(Item));
    item->id = id;
    strcpy(item->name, name);
    strcpy(item->description, description);
    item->type = type;
    item->subType = subType;
    item->uses = uses;
    item->value = value;
    return item;
}

//Chekcs if there is any item of the type received, if there is at least one then returns a random one of that type
Item * getRanItem()
{
    
    int listN = randIntLimits(0,2);
    if(itemsList[listN].amount < 1)
    {
        printf("There are no items of listN [%i] on [items.csv], please add at least one.", listN);
        exit(1);
    }
    Item * item = firstList(itemsList[listN].list);
    if(item == NULL)
    {
        printf("There are no items of listN [%i] on [items.csv], please add at least one.", listN);
        exit(1);
    }
    int i = randIntLimits(0, itemsList[listN].amount-1);
    for(int j = 0 ; j < i ; j++)
    {
        item = nextList(itemsList[listN].list);
        if(item == NULL)
        {
            printf("The item n° %i doesn't exist, going to the last one\n", j);
            item = prevList(itemsList[listN].list);
            break;
        }
    }
    item = createItem(item->id, item->name, item->description, item->type, item->subType, item->uses, item->value);
    return item;
}

//----------CHARACTER----------CHARACTER----------CHARACTER----------CHARACTER----------CHARACTER----------

// Generates a character with the name, race, level and stats
Character * createChara(Stats * stats, int level, char * name, int race)
{
    Character * newChar = (Character *) malloc (sizeof(Character));
    strcpy(newChar->name, name);
    newChar->race = race;
    newChar->level = level;
    newChar->experience = 0;
    newChar->expToLvlUp = level*5;
    newChar->state = 0;
    newChar->stats = stats;
    newChar->currentHP = stats->hp;
    newChar->weapons[0] = createWeapon("Bare hands", "Your own hands, you know them well.", 1, 0, 0, 1, 0, 0, 0, 0, 0);
    newChar->weapons[1] = NULL;
    newChar->weapons[2] = NULL;
    newChar->weapons[3] = NULL;
    newChar->currentWeapon = 0;
    newChar->items = createTreeMap(lowerThanInt);
    newChar->isPlayer = 0;
    return newChar;
}

//Creates a new character of the level received, with a random race and gives them a random weapon and a random item of type 1 or 2
Character * genRanChara(int level)
{
    int race = randIntLimits(1, racesFile->amount);
    Character * newChara = createChara(genRanCharaStats(level), level, (char*)getRaceString(race), race);
    newChara->experience = randIntLimits(0, newChara->expToLvlUp-1);
    giveWeapon(newChara, getRanWeapon(getRanWpnRarity(level)));
    chooseNextAvailableWeapon(newChara);
    giveItem(newChara, getRanItem(randIntLimits(0,5)));
    printf("The enemy got a [%s]!\n", newChara->weapons[newChara->currentWeapon]->name);
    return newChara;
}

//Asks for the name and the race the player wants, then generates stats and creates the character.
Character * createPlayerChara()
{
    printf("----------Character Creation----------\n");
    printf("What is your name?\n");
    char name[25];
    getchar();
    gets(name);
    printf("What are you?\n");
    for(int i = 1 ; i < racesFile->amount+1 ; i++)
    {
        if(i%5 == 0) printf("\n");
        printf("%i. [%s]  ", i, getRaceString(i));
    }
    printf("\n");
    int race;
    scanf("%i", &race);

    Stats * stats = genRanCharaStats(5);

    Character * newChar = createChara(stats, 1, name, race);
    newChar->isPlayer=1;
    return newChar;
}

//Shows only the important info of a character
void printShortChara(Character * chara)
{
    printf("[%s]", chara->name);
    printf(" HP[%i/%i]\n", chara->currentHP, chara->stats->hp);
    printf("Lvl [%i]\n", chara->level);
    printf("Exp [%i/%i]\n", chara->experience, chara->expToLvlUp);
}

//Shows the info of a character
void printChara(Character * chara)
{
    printf("Name  [%s]\n", chara->name);
    printf("Level [%i]\n", chara->level);
    printf("Exp: [%i/%i]\n", chara->experience, chara->expToLvlUp);
    printf("Race  [%s]\n", getRaceString(chara->race));
    printf("Stats:\n");
    printf("    HP  [%i/%i]\n", chara->currentHP, chara->stats->hp);
    printf("    Atk [%i]\n", chara->stats->atk);
    printf("    Spd [%i]\n", chara->stats->spd);
    printf("    Def [%i]\n", chara->stats->def);
    printf("    Res [%i]\n", chara->stats->res);
}

//Increases the level of a character by 1 and increases their stats randomly
void levelUp(Character * chara)
{
    if(chara->isPlayer == 1) printf("-----LEVEL UP!-----\n");
    else printf("-----THE ENEMY HAS LEVELED UP!!!-----\n");
    getchar();

    chara->level++;
    printf("Level %i -> %i\n", chara->level-1, chara->level);

    int chance = randIntLimits(0, 100);
    if(chance <= 65)
    {
        chara->stats->hp++;
        chara->currentHP++;
        if(chara->isPlayer == 1) printf("HP +1\n"); getchar();
    }

    chance = randIntLimits(0, 100);
    if(chance <= 60)
    {
        chara->stats->atk++;
        if(chara->isPlayer == 1) printf("Atk +1\n"); getchar();
    }

    chance = randIntLimits(0, 100);
    if(chance <= 40)
    {
        chara->stats->spd++;
        if(chara->isPlayer == 1) printf("Spd +1\n"); getchar();
    }

    chance = randIntLimits(0, 100);
    if(chance <= 40)
    {
        chara->stats->def++;
        if(chara->isPlayer == 1) printf("Def +1\n"); getchar();
    }

    chance = randIntLimits(0, 100);
    if(chance <= 50)
    {
        chara->stats->res++;
        if(chara->isPlayer == 1) printf("Res +1\n"); getchar();
    }
    if(chara->isPlayer == 1) printf("-------------------\n");
    else printf("They have gained some power, be careful...\n");

    chara->expToLvlUp+=5;
}

//Gives the amount received as experience to the chara, if it's equal or greater to the limit then levels up
void giveExperience(Character * chara, int exp)
{
    chara->experience += exp;
    if(chara->isPlayer == 1) printf("You gained %i exp\n", exp);
    while(chara->experience > chara->expToLvlUp)
    {
        chara->experience-=chara->expToLvlUp;
        levelUp(chara);
    }
}

int dialogueChoice(char * option1, char * option2)
{
    int a;
    printf("1. %s\n2. %s\n", option1, option2);
    scanf("%i", &a);
    if(a == 1) return 1;
    else return 0;
}

void dialogueEvent(Character * playerChara, RunManager * runManager)
{
    int r = randIntLimits(0,3);
    switch(r)
    {
        case 0:
        {
            getchar();
            printf("On your way to the next room you found a chest!\n");
            getchar();
            if(dialogueChoice("Open", "Skip") == 1)
            {
                printf("You opened it but nothing was inside...\n");
                getchar();
                printf("Maybe someone else openned it before you.\n");
                getchar();
                printf("You leave the room with a frown on the face.\n");
            }
            else printf("You skip the chest and head to the next room.\n");
            pressEnterToContinue();
            break;
            
        }
        case 1:
        {
            getchar();
            printf("Nothing interesting happened on your way to the next room...\n");
            pressEnterToContinue();
            break;
        }
        case 2:
        {
            getchar();
            printf("\"Good job little warrior.\"\n\"You may have won this battle, but there are many more next.\"\n\"Continue if you have a death wish, or retreat while you can... if you can...\"\n");
            getchar();
            giveExperience(playerChara, (5+(playerChara->expToLvlUp/5)));
            pressEnterToContinue();
            break;
        }
        case 3:
        {
            getchar();
            if(runManager->room == 5) printf("Scary growls can be heard from the door to the next floor.\n");
            else printf("Scary growls can be heard from the door to the next room.\n");
            pressEnterToContinue();
            break;
        }
    }
}

void itemEvent(Character * playerChara, RunManager * runManager)
{
    getchar();
    printf("After defeating the enemy you notice a chest!\n");
    getchar();
    if(dialogueChoice("Open", "Skip") == 1)
    {
        printf("You open it and an item is inside it!\n");
        Item * item = getRanItem();
        getchar();
        printf("You found a %s!\n", item->name);
        giveItem(playerChara, item);
        giveExperience(playerChara, 5);
        if(item->type == 1) runManager->score+= (item->value/20);
        else runManager->score+=item->value;
    }
    else printf("You skip the chest and head to the next room.\n");
    pressEnterToContinue();
}

void weaponEvent(Character * playerChara, RunManager * runManager)
{
    getchar();
    printf("After defeating the enemy you notice a chest!\n");
    getchar();
    if(dialogueChoice("Open", "Skip") == 1)
    {
        printf("You open it and a weapon is inside it!\n");
        Weapon * wpn = getRanWeapon(fmin(5, runManager->floor));
        getchar();
        printf("You found a %s!\n", wpn->name);
        giveWeapon(playerChara, wpn);
        giveExperience(playerChara, 5);
        runManager->score+=wpn->rarity;
    }
    else printf("You skip the chest and head to the next room.\n");
    pressEnterToContinue();
}

void trapEvent(Character * playerChara, RunManager * runManager)
{
    int r = randIntLimits(0,6);
    int i;
    switch(r)
    {
        case 0:
        {
            getchar();
            printf("On your way to the next room you found a chest!\n");
            getchar();
            if(dialogueChoice("Open", "Skip") == 1)
            {
                i = randIntLimits(playerChara->stats->hp/3, playerChara->stats->hp/2);
                playerChara->currentHP = fmax(1, (playerChara->currentHP - i));
                printf("You opened it but nothing was inside...\n");
                getchar();
                printf("WAIT IT HAS FANGS\n");
                getchar();
                printf("IT'S TRYING TO EAT YOU\n");
                getchar();
                printf("...\n");
                getchar();
                printf("You barely escape alive...\n");
                getchar();
                printf("You received %i damage from it!\n", i);
                getchar();
                printf("You should stab the next chest before opening it.\n");
                getchar();
                printf("Don't forget it!\n");
                giveExperience(playerChara, 6);
            }
            else printf("You skip the chest and head to the next room.\n");
            pressEnterToContinue();
            break;
        }
        case 1:
        {
            i = randIntLimits(playerChara->stats->def/4, playerChara->stats->def/3);
            playerChara->stats->def = fmax(0, (playerChara->stats->def - i));
            getchar();
            printf("You stepped on a trap!\n");
            getchar();
            printf("You got hit by... slime?\n");
            getchar();
            printf("It's desintegrating your clothes!\n");
            getchar();
            printf("Your Defense got reduced by %i!!!\n", i);
            getchar();
            giveExperience(playerChara, 3);
            pressEnterToContinue();
            break;
        }
        case 2:
        {
            i = randIntLimits(playerChara->stats->res/4, playerChara->stats->res/3);
            playerChara->stats->res = fmax(0, (playerChara->stats->res - i));
            getchar();
            printf("Entering the next room, you feel different...\n");
            getchar();
            printf("Weaker...\n");
            getchar();
            printf("Your resistance feels lower...\n");
            getchar();
            printf("It got reduced by %i!!!\n", i);
            pressEnterToContinue();
            break;
        }
        case 3:
        {
            i = randIntLimits(playerChara->stats->spd/4, playerChara->stats->spd/3);
            playerChara->stats->spd = fmax(0, (playerChara->stats->spd - i));
            getchar();
            printf("The gravity in from this room is stronger.\n");
            getchar();
            printf("Your body feels heavier from now on, making you slower.\n");
            getchar();
            printf("You are slowed by %i!!!\n", i);
            pressEnterToContinue();
            break;
        }
        case 4:
        {
            if(runManager->floor == 1) runManager->room = 0;
            else
            {
                i = (randIntLimits(1,5));
                if(runManager->room < i) runManager->floor--;
                runManager->room = i;
            }
            getchar();
            printf("You got lost???.\n");
            getchar();
            printf("You went back some rooms and now a new enemy is there, blocking your path...\n");
            pressEnterToContinue();
            break;
        }
        case 5:
        {
            i = (randIntLimits(1,15));
            runManager->room+=i;
            runManager->floor+=runManager->room/5;
            runManager->room = runManager->room%5;
            
            getchar();
            printf("You found a secret passage!\n");
            getchar();
            printf("You skipped some rooms!\n");
            pressEnterToContinue();
            break;
        }
        case 6:
        {
            i = randIntLimits(playerChara->stats->atk/4, playerChara->stats->atk/3);
            playerChara->stats->atk = fmax(0, (playerChara->stats->atk - i));
            getchar();
            printf("You remembered an embarrassing moment of your past!\n");
            getchar();
            printf("Your attacks became weaker by %i!!!\n", i);
            pressEnterToContinue();
            break;
        }
    }
}

//Triggers a random event
void randomEvent(Character * playerChara, RunManager * runManager)
{
    int op = randIntLimits(1,100);
    if(op > 70) dialogueEvent(playerChara, runManager);
    else if(op>40) weaponEvent(playerChara, runManager);
    else if(op>15) itemEvent(playerChara, runManager);
    else if(op>0) trapEvent(playerChara, runManager);
}

void lowerWeaponDurability(Character * chara)
{
    if(chara->currentWeapon != 0)
    {
        chara->weapons[chara->currentWeapon]->durability--;
        if(chara->weapons[chara->currentWeapon]->durability == 0)
        {
            printf("%s's weapon has broken!\n", chara->name);
            free(chara->weapons[chara->currentWeapon]);
            chara->weapons[chara->currentWeapon] = NULL;
            chooseNextAvailableWeapon(chara);
            if(chara->currentWeapon == 0) printf("Now they are unarmed!\n");
        }
    }
}

//Calculates the damage the Attacker would deald to the Defender
int damageCalc(Character * attacker, Character * defender)
{
    int totalAtk, dmgReduction, damageDealt;
    totalAtk = attacker->stats->atk + attacker->weapons[attacker->currentWeapon]->stats->atk;
    if(attacker->weapons[attacker->currentWeapon]->effectiveness == defender->race)
    {
        printf("The weapon is super effective!\n");
        totalAtk*=1.5;
    }
    switch(attacker->weapons[attacker->currentWeapon]->damageType)
    {
        case 0: dmgReduction = 0; break;
        case 1: dmgReduction = defender->stats->def; break;
        case 2: dmgReduction = defender->stats->res; break;
    }
    damageDealt = fmax(1, totalAtk - dmgReduction);
    if(defender->state == 2)
    {
        if(randIntLimits(1,10) == 1)
        {
            damageDealt = 0;
            printf("%s completely blocked the attack!!\n", defender->name);
        }
        else
        {
            damageDealt = damageDealt/2;
            printf("%s blocked part of the attack!\n", defender->name);
        }
    }
    printf("%s received %i damage.\n",defender->name , damageDealt);
    giveExperience(attacker, fmax(1, damageDealt/3));

    return damageDealt;
}

//Returns 0 if the enemy is alive, and 1 if the enemy died because of the attack
int attack(Character * attacker, Character * defender)
{
    defender->currentHP = fmax(0, defender->currentHP - damageCalc(attacker, defender));
    pressEnterToContinue();

    if(defender->state == 2)
    {
        defender->state = 0;
        printf("%s is not defending anymore.\n", defender->name);
        giveExperience(defender, 1);
        pressEnterToContinue();
    }
    lowerWeaponDurability(attacker);
    attacker->state = 0;
    attacker->currentHP+=attacker->weapons[attacker->currentWeapon]->stats->hp;
    if(attacker->weapons[attacker->currentWeapon]->stats->hp > 0) printf("%s healed themselves %i HP.\n", attacker->name, attacker->weapons[attacker->currentWeapon]->stats->hp);
    if(attacker->weapons[attacker->currentWeapon]->stats->hp < 0) printf("%s received %i damage by the weapon.\n", attacker->name, attacker->weapons[attacker->currentWeapon]->stats->hp*-1);

    if(defender->currentHP == 0)
    {
        system("cls");
        printf("%s has been defeated!\n", defender->name);
        getchar();
        giveExperience(attacker, defender->level*6);
        pressEnterToContinue();
        return 1;
    }
    return 0;
}

//Player's turn
int playerPhase(Character * playerChara, Character * enemyChara, RunManager * runManager)
{
    int op, pass = 0;
    Pair * pair;
    do
    {
        printf("-----Your turn!-----\n");
        printf(">>> %s HP[%i/%i] Lvl %i Exp[%i/%i]\n", playerChara->name, playerChara->currentHP, playerChara->stats->hp, playerChara->level, playerChara->experience, playerChara->expToLvlUp);
        printf("1. Attack\n");
        printf("2. Defend\n");
        printf("3. Use Item\n");
        printf("4. Show Player Info (Does not count as an action)\n");
        printf("5. Check enemy info\n");
        printf("0. Save and Exit\n");
        scanf("%i", &op);
        switch(op)
        {
            case 0:
            {
                if(saveRun(playerChara, enemyChara, runManager) == 0) return 2;
                break;
            }
            case 1:
            {
                pass = 1;
                chooseNextAvailableWeapon(playerChara);
                if(playerChara->currentWeapon != 0)
                {
                    printf("Choose a weapon to attack with\n");
                    for(int w = 1 ; w < 4 ; w++)
                    {
                        if(playerChara->weapons[w] != NULL) printf("%i. %s\n", w, playerChara->weapons[w]->name);
                    }
                    while(1)
                    {
                        scanf("%i", &playerChara->currentWeapon);
                        if(playerChara->weapons[playerChara->currentWeapon] == NULL || playerChara->currentWeapon > 3 || playerChara->currentWeapon < 1) printf("Not valid\n");
                        else break;
                    }
                }
                system("cls");
                printf("%s attacks with the %s!\n", playerChara->name, playerChara->weapons[playerChara->currentWeapon]->name);
                playerChara->state = 1;
                runManager->score+=2;
                getchar();
                if(attack(playerChara, enemyChara) == 1) return 1;
                system("cls");
                break;
            }
            case 2:
            {
                pass = 1;
                system("cls");
                printf("%s is defending the next attack!\n", playerChara->name);
                playerChara->state = 2;
                runManager->score++;
                getchar();
                pressEnterToContinue();
                system("cls");
                break;
            }
            case 3:
            {
                if(checkIfEmptyItems(playerChara) == 0)
                {
                    printf("You don't have any items and wasted your turn!\n");
                    break;
                }
                printf("Choose an Item to use (type the ID).\n");
                showItems(playerChara);
                while(1)
                {
                    scanf("%i", &op);
                    if(usePlayerItem(playerChara, enemyChara, op) == 1) break;
                }
                pass = 1;
                break;
            }
            case 4:
            {
                system("cls");
                printf("------------Your current Info----------\n");
                printChara(playerChara);
                printf("\n-------------Your Weapons--------------\n");
                showWeapons(playerChara);
                printf("\n--------------Your Items---------------\n");
                showItems(playerChara);
                
                getchar();
                pressEnterToContinue();
                system("cls");
                break;
            }
            case 5:
            {
                pass = 1;
                system("cls");
                printf("---------------Enemy Info--------------\n");
                printChara(enemyChara);
                printf("\n------------Enemy's Weapons------------\n");
                showWeapons(enemyChara);
                printf("\n-------------Enemy's Items-------------\n");
                showItems(enemyChara);

                getchar();
                pressEnterToContinue();
                system("cls");
                break;
            }
        }
    } while (pass == 0);
   return 0;
}

//Automatically makes a choice for the enemy character for their act. If attack() returns 1, this function also returns 1.
int aiPhase(Character * enemyChara, Character * playerChara)
{
    int op;
    printf("-----Enemy turn!-----\n");
    pressEnterToContinue();
    system("cls");
    int chance = randIntLimits(1,100);
    if(enemyChara->currentHP >= enemyChara->currentHP/2)
    {
        if(chance == 1) op = 3;
        else op = 1;
    }
    else if(enemyChara->currentHP < enemyChara->currentHP/10)
    {
        if(chance > 49) op = 2;
        else op = 3;
    }
    else
    {
        if(chance > 49) op = 1;
        else if(chance > 19) op = 2;
        else if(chance > 10) op = 3;
        else op = 4;
    }
    switch(op)
    {
        case 1:
        {
            printf("%s attacks!\n", enemyChara->name);
            enemyChara->state = 1;
            if(attack(enemyChara, playerChara) == 1) return 1;
            break;
        }
        case 2:
        {
            printf("%s is defending the next attack!\n", enemyChara->name);
            enemyChara->state = 2;
            break;
        }
        case 3:
        {
            printf("%s is using an item!\n", enemyChara->name);
            useAiItem(enemyChara, playerChara);
            break;
        }
        case 4: return 2;
    }
    pressEnterToContinue();
    system("cls");
    return 0;
}

//Saves the score on the Highscores file
void saveScore(Character * chara, RunManager * runManager)
{
    highScoresFile = fopen("./resources/highscores.csv", "a");
    if(highScoresFile == NULL)
    {
        printf("ERROR: The file [highscores.csv] is corrupted.\n");
        pressEnterToContinue();
        return;
    }
    int score = runManager->score + (runManager->room) + (runManager->floor*5);
    printf("Your score with %s was [%i]\n", chara->name, score);
    fseek(highScoresFile, 0, SEEK_END);
    fprintf(highScoresFile, "\"%s\",%i\n", chara->name, score);
    fclose(highScoresFile);
}

//Clears the save.csv file
void deleteSave()
{
    FILE * saveFile = fopen("./save/save.csv", "w");
    fclose(saveFile);
}

//The actual game
void play(RunManager * runManager, Character * playerChara, Character * enemyChara)
{
    int battleResults, maxTurns = 10;
    printf("\n");
    while(runManager->state != 0)
    {
        if(runManager->turn > maxTurns) runManager->state = 4;
        switch(runManager->state)
        {
            case 1:
            {
                // Before a new combat, an Enemy has to be generated
                pressEnterToContinue();
                system("cls");
                printf("-----Before combat-----\n");
                runManager->room++, runManager->score++;
                if(runManager->room%11 == 0) runManager->floor++, runManager->room = 0, runManager->score+=10;

                printf("Floor [%i] Room [%i]\n", runManager->floor, runManager->room);
                pressEnterToContinue();
                system("cls");

                printf("Generating new enemy\n");
                pressEnterToContinue();
                enemyChara = genRanChara(fmax(1,(int)((randFloatLimits(80, 120)/100)*playerChara->level)));

                //Entering the Combating runManager->state
                runManager->state = 2;
                break;
            }
            case 2:
            {
                pressEnterToContinue();
                system("cls");
                runManager->turn++;
                printf("-----Turn %i-----\n", runManager->turn);
                pressEnterToContinue();
                if(playerChara->stats->spd > enemyChara->stats->spd)
                {
                    battleResults = playerPhase(playerChara, enemyChara, runManager);
                    if(battleResults == 2) return;
                    else if(battleResults == 1)
                    {
                        printf("The foe has been defeated!\n");
                        runManager->state = 3;
                        runManager->score+=10;
                        pressEnterToContinue();
                        continue;
                    }
                    
                    if(aiPhase(enemyChara, playerChara) == 1)
                    {
                        printf("You have been defeated...\n");
                        runManager->state = 4;
                        pressEnterToContinue();
                        continue;
                    }
                }
                else if(playerChara->stats->spd <= enemyChara->stats->spd)
                {
                    battleResults = aiPhase(enemyChara, playerChara);
                    if(battleResults == 1)
                    {
                        printf("You have been defeated...\n");
                        runManager->state = 4;
                        pressEnterToContinue();
                        continue;
                    }
                    else if(battleResults == 2)
                    {
                        printf("The foe run away!\n");
                        printf("They couldn't handle your coolness.\n");
                        runManager->state = 3;
                        runManager->score+=10;
                        pressEnterToContinue();
                        continue;
                    }
                    battleResults = playerPhase(playerChara, enemyChara, runManager);
                    if(battleResults == 2) return;
                    else if(battleResults == 1)
                    {
                        printf("The foe has been defeated!\n");
                        runManager->state = 3;
                        runManager->score+=12;
                        pressEnterToContinue();
                        continue;
                    }
                }
                break;
            }
            case 3:
            {
                system("cls");
                randomEvent(playerChara, runManager);
                system("cls");
                freeCharacter(enemyChara);
                printf("-----After combat-----\n");
                printf("Your current state:\n");
                printShortChara(playerChara);
                runManager->turn = 0;
                runManager->state = 1;
                break;
            }
            case 4:
            {
                deleteSave();
                system("cls");
                if(runManager->turn > maxTurns) printf("You took so long to defeat the enemy that more enemies came and you coulnd't against all of them.\n");
                printf("Game over");
                pressEnterToContinue();
                saveScore(playerChara, runManager);
                freeCharacter(enemyChara);
                freeCharacter(playerChara);
                runManager->state = 0;
                return;
            }

        }
    }
}

//----------LOAD----------LOAD----------LOAD----------LOAD----------LOAD----------LOAD----------
//Creates items for every line read in the file, then inserts them in the character's inventory
void loadItems(Character * chara, FILE * file, char * l)
{
    while(fgets(l, 1023, file) != NULL)
    {
        Item * item = createItem(atoi(getCSVField(l, 0)), (char*)getCSVField(l, 1), (char*)getCSVField(l,2), atoi(getCSVField(l,3)), atoi(getCSVField(l,4)), atoi(getCSVField(l,5)), atoi(getCSVField(l,6)));
        insertTreeMap(chara->items, &item->id, item);
    }
}

//Creates a character and their weapons from the save file read.
Character * loadChara(FILE * saveFile, char * l)
{
    fgets(l, 1023, saveFile);
    Character * newChar = (Character *) malloc (sizeof(Character));
    strcpy(newChar->name, getCSVField(l,0));
    newChar->race = atoi(getCSVField(l,1));
    newChar->level = atoi(getCSVField(l,2));
    newChar->experience = atoi(getCSVField(l,3));
    newChar->expToLvlUp = atoi(getCSVField(l,4));
    newChar->state = atoi(getCSVField(l,5));
    newChar->stats = (Stats*) malloc (sizeof(Stats));
    newChar->currentHP = atoi(getCSVField(l,6));
    newChar->currentWeapon = atoi(getCSVField(l,7));
    newChar->stats->hp = atoi(getCSVField(l,8));
    newChar->stats->atk = atoi(getCSVField(l,9));
    newChar->stats->spd = atoi(getCSVField(l,10));
    newChar->stats->def = atoi(getCSVField(l,11));
    newChar->stats->res = atoi(getCSVField(l,12));
    newChar->weapons[0] = createWeapon("Bare hands", "Your own hands, you know them well.", 1, 0, 0, 1, 0, 0, 0, 0, 0);
    for(int w = 1 ; w < 4 ; w++)
    {
        fgets(l, 1023, saveFile);
        if(strcmp(l, "NULL\n") == 0) newChar->weapons[w] = NULL;
        else newChar->weapons[w] = createWeapon((char*)getCSVField(l, 0), (char*)getCSVField(l, 1), atoi(getCSVField(l,2)), atoi(getCSVField(l,3)), atoi(getCSVField(l,4)), atoi(getCSVField(l,5)), atoi(getCSVField(l,6)), atoi(getCSVField(l,7)), atoi(getCSVField(l,8)), atoi(getCSVField(l,9)), atoi(getCSVField(l,10)));
    }
    newChar->items = createTreeMap(lowerThanInt);
    newChar->isPlayer = 0;
    return newChar;
}

//Loads a run by reading the save files
void loadRun(RunManager * runManager)
{
    system("cls");
    printf("Opening file\n");
    FILE * file = fopen("./save/save.csv", "r");
    if(file == NULL)
    {
        printf("There isn't any save file.\n");
        return;
    }
    char l[1024];
    fseek(file, 0, SEEK_SET);
    printf("Checking if the file is empty\n");
    if(fgets(l, 1023, file) == NULL)
    {
        printf("There isn't any save file.\n");
        return;
    }
    printf("Loading Run Manager\n");
    runManager = (RunManager*) malloc (sizeof(RunManager));
    runManager->floor = atoi(getCSVField(l,0));
    runManager->room = atoi(getCSVField(l,1));
    runManager->turn = atoi(getCSVField(l,2));
    runManager->score = atoi(getCSVField(l,3));

    printf("Loading Player Character\n");
    Character * playerChara = loadChara(file, l);
    printf("Loading Enemy Character\n");
    Character * enemyChara = loadChara(file, l);
    fclose(file);
    playerChara->isPlayer = 1;
    enemyChara->isPlayer = 0;

    printf("Loading Player Items\n");
    file = fopen("./save/playerItems.csv", "r");
    if(file != NULL)
    {
        loadItems(playerChara, file, l);
        fclose(file);
    }

    printf("Loading Enemy Items\n");
    file = fopen("./save/enemyItems.csv", "r");
    if(file != NULL)
    {
        loadItems(enemyChara, file, l);
        fclose(file);
    }

    runManager->turn = 0;
    runManager->state = 2;
    printf("Loaded...\n");
    play(runManager, playerChara, enemyChara);
}

//Starts a new run from the start
void newRun(RunManager * runManager)
{
    system("cls");
    runManager = (RunManager*) malloc (sizeof(RunManager));
    runManager->room = 0;
    runManager->floor = 1;
    runManager->score = 0;
    runManager->turn = 0;
    runManager->state = 1;

    Character * enemyChara;
    Character * playerChara = createPlayerChara();

    system("cls");
    pressEnterToContinue();
    printf("\nYour character's (randomly generated) stats:\n");
    printChara(playerChara);
    giveWeapon(playerChara, getRanWeapon(1));
    playerChara->currentWeapon = 1;
    printf("\nYou brought a %s as a weapon!\n", playerChara->weapons[playerChara->currentWeapon]->name);

    printf("Giving you a random item!\n");
    giveItem(playerChara, getRanItem(randIntLimits(0,5)));
    play(runManager, playerChara, enemyChara);
}

//Shows the HighScores History
void showHighScores()
{
    highScoresFile = fopen("./resources/highscores.csv", "r");
    if(highScoresFile == NULL)
    {
        printf("ERROR: The file [highscores.csv] is missing.\n");
        return;
    }
    fseek(highScoresFile, 0, SEEK_SET);
    char l[1024];
    if(fgets(l, 1023, highScoresFile) == NULL)
    {
        printf("There are no HighScores yet, go and be the first!\n");
        return;
    }
    
    char n[201];
    int s;
    printf("-----------------HIGHSCORES-----------------\n");
    do{
        strcpy(n, getCSVField(l, 0));
        s = atoi(getCSVField(l, 1));
        printf("%-25s %18i\n", n, s);
    }while(fgets(l, 24, highScoresFile) != NULL);
    fclose(highScoresFile);
    printf("Type \"Delete HighScores\" to clean ALL the HighScores in history.\n(This action can not be reverted)\n");
    
    char o[30];
    gets(o);
    if(strcmp("delete highscores", stringToLower(o)) == 0)
    {
        highScoresFile = fopen("./resources/highscores.csv", "w");
        fclose(highScoresFile);
        printf("HighScores have been deleted.\n");
    }
}

//Reads the items file and creates the items on the list
void readItems()
{
    FILE * itemsFile = fopen("./resources/items.csv", "r");
    if(itemsFile == NULL)
    {
        printf("ERROR: The file [items.csv] is missing.");
        exit(1);
    }
    printf("mallocing items**\n");
    itemsList = (ItemsListNode *) malloc (3*sizeof(ItemsListNode));
    int n;
    printf("creating the 3 lists\n");
    for(n = 0 ; n < 3 ; n++)
    {
        itemsList[n].list = createList();
        if(itemsList[n].list == NULL)
        {
            printf("There was an error while reading the [items.csv] file.");
            exit(1);
        }
        printf("items list [%i] created\n", n);
        itemsList[n].amount = 0;
    }
    printf("All lists of items created\n");
    char l[1024];
    n = 0;
    Item * newItem;
    int listN;
    while(fgets(l, 1023, itemsFile) != NULL)
    {
        if(n == 0) n = 1;
        else
        {
            printf("Creating Item id[%i]\n", n);
            newItem = createItem(n, (char*)getCSVField(l, 0), (char*)getCSVField(l, 1), atoi(getCSVField(l,2)), atoi(getCSVField(l,3)), atoi(getCSVField(l,4)), atoi(getCSVField(l,5)));
            printf("----------------------------------------\n");
            printItem(newItem);
            printf("----------------------------------------\n");
            printf("Item created, pushing into list\n");
            if(newItem->type > 2) listN = 2;
            else if(newItem->type > 0) listN = 1;
            else listN = 0;
            pushBack(itemsList[listN].list, newItem);
            printf("Pushed into list [%i]\n", listN);
            itemsList[listN].amount++;
            n++;
        }
    }
    fclose(itemsFile);
}

//Reads the weapons file and creates the weapons on the list
void readWeapons()
{
    FILE * weaponsFile = fopen("./resources/weapons.csv", "r");
    if(weaponsFile == NULL)
    {
        printf("ERROR: The file [weapons.csv] is missing.");
        exit(1);
    }
    printf("mallocing weaponsList**\n");
    weaponsList = (WeaponsListNode *) malloc (6*sizeof(WeaponsListNode));
    int i;
    printf("creating the 6 lists\n");
    for(i = 0 ; i < 6 ; i++)
    {
        weaponsList[i].list = createList();
        if(weaponsList[i].list == NULL)
        {
            printf("There was an error while reading the [weapons.csv] file.");
            exit(1);
        }
        printf("Weapon list [%i] created\n", i);
        weaponsList[i].amount = 0;
    }
    printf("All lists of weapons created\n");
    char l[1024];
    i = 0;
    while(fgets(l, 1023, weaponsFile) != NULL)
    {
        if(i == 0) i = 1;
        else
        {
            Weapon * newWeapon = createWeapon((char*)getCSVField(l, 0), (char*)getCSVField(l, 1), atoi(getCSVField(l,2)), atoi(getCSVField(l,3)), atoi(getCSVField(l,4)), atoi(getCSVField(l,5)), atoi(getCSVField(l,6)), atoi(getCSVField(l,7)), atoi(getCSVField(l,8)), atoi(getCSVField(l,9)), atoi(getCSVField(l,10)));

            //printWeapon(newWeapon);
            pushBack(weaponsList[newWeapon->rarity-1].list, newWeapon);
            weaponsList[newWeapon->rarity-1].amount++;
        }
    }
    fclose(weaponsFile);
}

//Opens the races file and calculates the amount of races in the file
void openRaces()
{
    racesFile = (RacesFile *) malloc (sizeof(RacesFile));
    if(racesFile == NULL)
    {
        printf("There was an error reading the [races.csv] file.");
        exit(1);
    }
    racesFile->file = fopen("./resources/races.csv", "r");
    if(racesFile->file == NULL)
    {
        printf("ERROR: The file [races.csv] is missing.");
        exit(1);
    }
    racesFile->amount = 0;
    char l[1024];
    if(fgets(l, 1023, racesFile->file) != NULL)
    {
        int i = 0;
        while(getCSVField(l,i) != NULL)
        {
            racesFile->amount++;
            i++;
        }
    }
    else printf("There was an error while reading the file [races.csv]");
}

//Opens all necesary files
void openAllFiles()
{
    readItems();
    printf("[items.csv] read successfully\n");

    readWeapons();
    printf("[weapons.csv] read successfully\n");

    openRaces();
    printf("[races.csv] read successfully\n");

    printf("[highscores.csv] read successfully\n");
}

//Main menu and calls openAllFiles
int main()
{
    srand(time(NULL));
    int op = 0, c;
    RunManager * runManager;
    openAllFiles();
    printf("------------------Start------------------\n");
    pressEnterToContinue();

    while(op != 4)
    {
        system("cls");
        printf("\n--------------------MENU--------------------\n");
        printf("1. New run\n");
        printf("2. Continue\n");
        printf("3. Highscores\n");
        printf("4. Exit\n");
        printf("--------------------------------------------\n");
        printf("Input an option:\n");
        scanf("%i", &op);
        getchar();
        switch(op)
        {
            case 1:
            {
                printf("If there is another run in progress it will be deleted...\nAre you sure you want to start from zero?\n1. Start\n2. Return\n");
                scanf("%i", &c);
                if(c == 1) newRun(runManager);
                pressEnterToContinue();
                break;
            }
            case 2:
            {
                system("cls");
                printf("Loading run...\n");
                loadRun(runManager); 
                pressEnterToContinue();
                break;
            }
            case 3:
            {
                system("cls");
                showHighScores();
                pressEnterToContinue();
                break;
            }
            case 4:
            {
                fclose(racesFile->file);
                printf("Exiting the game...");
                break;
            }
        }
    }
    return 0;
}
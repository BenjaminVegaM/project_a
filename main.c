#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "list.h"

typedef struct
{
    int hp;
    int atk;
    int spd;
    int def;
    int res;
} Stats;

typedef struct
{
    char name[40];
    char description[200];
    int rarity;
    int currentDurability;
    int maxDurability;
    int effectiveness; //same number as race
    int damageType;
    Stats * stats; //increase in char stats
} Weapon;

typedef struct
{
    int isPlayer; //1 Player, 0 otherwise
    char name[25];
    int race; //a number that represents the race
    int level;
    int experience;
    int expToLvlUp;
    int state;
    int currentHP;
    Stats * stats;
    Weapon * weapons[4];
    int currentWeapon;
} Character;

typedef struct 
{
    Weapon * weapon;
    int id;
    int used;//If it's an epic or higher rarity weapon, there can only be one of those per run. 0 if they are infinite, 1 if they are limited and 2 if they already appeared.
} WeaponNode;

typedef struct 
{
    List * list;
    int amount;
} WeaponListNode;

typedef struct
{
    FILE * file;
    int amount;
}RacesFile;

typedef struct
{
    int floor;
    int room;
    int turn;
    int score;
}RunManager;


//Global Variable
WeaponListNode * weaponsList;
RacesFile * racesFile;
FILE * highScoresFile;

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
const char *getCSVField (char * tmp, int k) {

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

//Funcion que requiere presionar enter para proseguir, sirve para generar una pausa.
void pressEnterToContinue()
{
    printf("...");
    char enter;
    while((enter = getchar()) != '\r' && enter != '\n');
}

int saveRun(Character * chara1, Character * chara2, RunManager * runManager)
{
    printf("Saving the game...\n");
    FILE * saveFile = fopen("save.csv", "w");
    if(saveFile == NULL)
    {
        printf("There was an error opening the file [save.csv], please check the game files before trying again.\n(If necesary please delete the file, a new one will be created)\n");
        return 1;
    }
    int i;
    printf("Saving Numbers\n");
    fprintf(saveFile, "Test\nuwu");

    /*printf("Saving Player Character\n");
    //Player chara
    fprintf(saveFile, "%s,%s,%i,%i,%i,%i,%i,%i,%i\n", chara1->name, chara1->race, chara1->level, chara1->experience, chara1->expToLvlUp, chara1->state, chara1->currentHP, chara1->currentWeapon);
    fprintf(saveFile, "%i,%i,%i,%i,%i\n", chara1->stats->hp, chara1->stats->atk, chara1->stats->spd, chara1->stats->def, chara1->stats->res);
    
    for(i = 1 ; i < 4 ; i++)
    {
        if(chara1->weapons[i] == NULL)
        {
            fprintf(saveFile,"NULL\n");
        }
        else fprintf(saveFile, "%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\n", chara1->weapons[i]->name, chara1->weapons[i]->description, chara1->weapons[i]->rarity, chara1->weapons[i]->currentDurability, chara1->weapons[i]->maxDurability, chara1->weapons[i]->effectiveness, chara1->weapons[i]->damageType, chara1->weapons[i]->stats->hp, chara1->weapons[i]->stats->atk, chara1->weapons[i]->stats->spd, chara1->weapons[i]->stats->def, chara1->weapons[i]->stats->res);
    }

    printf("Saving Current Enemy Character\n");
    //Current Enemy chara
    fprintf(saveFile, "%s,%s,%i,%i,%i,%i,%i,%i,%i\n", chara2->name, chara2->race, chara2->level, chara2->experience, chara2->expToLvlUp, chara2->state, chara2->currentHP, chara2->currentWeapon);
    fprintf(saveFile, "%i,%i,%i,%i,%i\n", chara2->stats->hp, chara2->stats->atk, chara2->stats->spd, chara2->stats->def, chara2->stats->res);
    
    for(i = 1 ; i < 4 ; i++)
    {
        if(chara2->weapons[i] == NULL)
        {
            fprintf(saveFile, "NULL\n");
        }
        else fprintf(saveFile, "%s,%s,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\n", chara2->weapons[i]->name, chara2->weapons[i]->description, chara2->weapons[i]->rarity, chara2->weapons[i]->currentDurability, chara2->weapons[i]->maxDurability, chara2->weapons[i]->effectiveness, chara2->weapons[i]->damageType, chara2->weapons[i]->stats->hp, chara2->weapons[i]->stats->atk, chara2->weapons[i]->stats->spd, chara2->weapons[i]->stats->def, chara2->weapons[i]->stats->res);
    }*/

    printf("Game Saved!\n");
    fclose(saveFile);

    return 0;
}

// Generates random stats based on the Level of the character
Stats * genRanCharStats(int level)
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

void printWeapon(Weapon * weapon)
{
    printf("Name: [%s]\n", weapon->name);
    printf("Description: [%s]\n", weapon->description);
    printf("Rarity: [%s]\n", getRarityString(weapon->rarity));
    printf("Durability: [%i/%i]\n", weapon->currentDurability, weapon->maxDurability);
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

Weapon * createWeapon(char * name, char * description, int rarity, int durability, int effectiveness, int damageType, int hp, int atk, int spd, int def, int res)
{
    Weapon * weapon = (Weapon*) malloc (sizeof(Weapon));
    strcpy(weapon->name, name);
    strcpy(weapon->description, description);
    weapon->rarity = rarity;
    weapon->maxDurability = durability;
    weapon->currentDurability = durability;
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
    weapon = createWeapon(weapon->name, weapon->description, weapon->rarity, weapon->maxDurability, weapon->effectiveness, weapon->damageType, weapon->stats->hp, weapon->stats->atk, weapon->stats->spd, weapon->stats->def, weapon->stats->res);
    return weapon;
}

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
            if(w == 4) printf("You left behind the %s.\nMaybe it wasn't that good.\n", weapon->name);
            else
            {
                printf("You threw the %s.\nJust don't regret it later.\n", chara->weapons[w]->name);
                free(chara->weapons[w]);
                chara->weapons[w] = weapon;
            }
        }
    }
}

void chooseNextAvailableWeapon(Character * chara)
{
    int i = 3;
    while(chara->weapons[i] == NULL) i--;
    chara->currentWeapon = i;
}

// Generates a character with the stats
Character * generateChara(Stats * stats, int level, char * name, int race)
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
    newChar->isPlayer = 0;
    return newChar;
}

Character * genRanChara(int level)
{
    int race = randIntLimits(1, racesFile->amount);
    Character * newChara = generateChara(genRanCharStats(level), level, (char*)getRaceString(race), race);
    newChara->experience = randIntLimits(0, newChara->expToLvlUp-1);
    giveWeapon(newChara, getRanWeapon(getRanWpnRarity(level)));
    chooseNextAvailableWeapon(newChara);
    printf("The enemy got a [%s]!\n", newChara->weapons[newChara->currentWeapon]->name);
    return newChara;
}

Character * createChara()
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

    Stats * stats = genRanCharStats(3);

    Character * newChar = generateChara(stats, 1, name, race);
    return newChar;
}

void printShortChara(Character * chara)
{
    printf("[%s]", chara->name);
    printf(" HP[%i/%i]\n", chara->currentHP, chara->stats->hp);
    printf("Lvl [%i]\n", chara->level);
    printf("Exp [%i/%i]\n", chara->experience, chara->expToLvlUp);
}

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

void dialogueEvent(Character * chara, RunManager * runManager)
{
    int r = randIntLimits(0,3);
    switch(r)
    {
        case 0:
        {
            getchar();
            printf("On your way to the next room you found a chest!\n");
            getchar();
            printf("You opened it but nothing was inside...\n");
            getchar();
            printf("Maybe someone else openned it before you.\n");
            getchar();
            printf("You leave the room with a frown on the face.\n");
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
            pressEnterToContinue();
            break;
        }
        case 3:
        {
            getchar();
            printf("a\n");
            pressEnterToContinue();
            break;
        }
    }
}

void itemEvent(Character * chara, RunManager * runManager)
{
    printf("There should be an Item here.\n");
    pressEnterToContinue();
}

void weaponEvent(Character * chara, RunManager * runManager)
{
    getchar();
    printf("After defeating the enemy you notice a chest!\n");
    getchar();
    printf("You open it and a weapon is inside it!\n");
    Weapon * wpn = getRanWeapon(fmin(5, runManager->floor));
    getchar();
    printf("You found a %s!\n", wpn->name);
    giveWeapon(chara, wpn);
    pressEnterToContinue();
}

void trapEvent(Character * chara, RunManager * runManager)
{
    int r = randIntLimits(0,4);
    int i;
    switch(r)
    {
        case 0:
        {
            i = randIntLimits(chara->stats->hp/3, chara->stats->hp/2);
            chara->currentHP = fmax(1, (chara->currentHP - i));
            getchar();
            printf("On your way to the next room you found a chest!\n");
            getchar();
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
            printf("Don't forget it! Or you already did?\n");
            pressEnterToContinue();
            break;
        }
        case 1:
        {
            i = randIntLimits(chara->stats->def/4, chara->stats->def/3);
            chara->stats->def = fmax(0, (chara->stats->def - i));
            getchar();
            printf("You stepped on a trap!\n");
            getchar();
            printf("You got hit by... slime?\n");
            getchar();
            printf("It's desintegrating your clothes!\n");
            getchar();
            printf("Your Defense got reduced by %i!!!\n", i);
            pressEnterToContinue();
            break;
        }
        case 2:
        {
            i = randIntLimits(chara->stats->res/4, chara->stats->res/3);
            chara->stats->res = fmax(0, (chara->stats->res - i));
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
            i = randIntLimits(chara->stats->spd/4, chara->stats->spd/3);
            chara->stats->spd = fmax(0, (chara->stats->spd - i));
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
            if(runManager->floor == 1)
            {
                runManager->room = 0;
            }
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
        }
        //case 4: lost a weapon (create a function for this and put it on the "if inventory full")
    }
}

void randomEvent(Character * chara, RunManager * runManager)
{
    int op = randIntLimits(1,100);
    if(op > 70)
    {
        dialogueEvent(chara, runManager);
    }
    else if(op>40)
    {
        weaponEvent(chara, runManager);
    }
    else if(op>15)
    {
        itemEvent(chara, runManager);
    }
    else if(op>0)
    {
        trapEvent(chara, runManager);
    }
}

void lowerWeaponDurability(Character * chara)
{
    if(chara->currentWeapon != 0)
    {
        chara->weapons[chara->currentWeapon]->currentDurability--;
        if(chara->weapons[chara->currentWeapon]->currentDurability == 0)
        {
            printf("%s's weapon has broken!\n", chara->name);
            free(chara->weapons[chara->currentWeapon]);
            chara->weapons[chara->currentWeapon] = NULL;
            chooseNextAvailableWeapon(chara);
            if(chara->currentWeapon == 0) printf("Now they are unarmed!\n");
        }
    }
}

void levelUp(Character * chara)
{
    if(chara->isPlayer == 1) printf("-----LEVEL UP!-----\n");
    else printf("-----THE ENEMY HAS LEVELED UP!!!-----\n");

    chara->level++;
    printf("Level %i -> %i\n", chara->level-1, chara->level);

    int chance = randIntLimits(0, 100);
    if(chance <= 65)
    {
        chara->stats->hp++;
        chara->currentHP++;
        if(chara->isPlayer == 1) printf("HP +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 60)
    {
        chara->stats->atk++;
        if(chara->isPlayer == 1) printf("Atk +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 40)
    {
        chara->stats->spd++;
        if(chara->isPlayer == 1) printf("Spd +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 40)
    {
        chara->stats->def++;
        if(chara->isPlayer == 1) printf("Def +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 50)
    {
        chara->stats->res++;
        if(chara->isPlayer == 1) printf("Res +1\n");
    }
    if(chara->isPlayer == 1) printf("-------------------\n");
    else printf("They have gained some power, be careful...\n");

    chara->expToLvlUp+=5;
}

void giveExperience(Character * chara, int exp)
{
    chara->experience += exp;
    printf("%s gained %i exp\n", chara->name, exp);
    if(chara->experience > chara->expToLvlUp)
    {
        chara->experience-=chara->expToLvlUp;
        levelUp(chara);
    }
}

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
        case 0: dmgReduction = 0;
        case 1: dmgReduction = defender->stats->def; break;
        case 2: dmgReduction = defender->stats->res; break;
    }
    damageDealt = fmax(1, totalAtk - dmgReduction);
    if(defender->state == 2)
    {
        damageDealt = damageDealt/2;
        if(randIntLimits(1,10) == 1)
        {
            damageDealt = 0;
            printf("%s completely blocked the attack!!\n", defender->name);
        }
        else
        {
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
        giveExperience(attacker, defender->level*6);
        pressEnterToContinue();
        return 1;
    }
    return 0;
}

int playerPhase(Character * chara1, Character * chara2, int score, RunManager * runManager)
{
    int op;
    printf("-----Your turn!-----\n");
    do
    {
        printf("1. Attack\n");
        printf("2. Defend\n");
        printf("3. Show Player Info (Does not count as an action)\n");
        printf("4. Check enemy info\n");
        printf("0. Save and Exit\n");
        scanf("%i", &op);
        switch(op)
        {
            case 0:
            {
                if(saveRun(chara1, chara2, runManager) == 0) return 2;
                break;
            }
            case 1:
            {
                if(chara1->currentWeapon != 0)
                {
                    printf("Choose a weapon to attack with\n");
                    for(int w = 1 ; w < 4 ; w++)
                    {
                        if(chara1->weapons[w] != NULL) printf("%i. %s\n", w, chara1->weapons[w]->name);
                    }
                    while(1)
                    {
                        scanf("%i", &chara1->currentWeapon);
                        if(chara1->weapons[chara1->currentWeapon] == NULL || chara1->currentWeapon > 3 || chara1->currentWeapon < 1) printf("Not valid\n");
                        else break;
                    }
                }
                system("cls");
                printf("%s attacks with the %s!\n", chara1->name, chara1->weapons[chara1->currentWeapon]->name);
                chara1->state = 1;
                score+=2;
                getchar();
                if(attack(chara1, chara2) == 1) return 1;
                system("cls");
                break;
            }
            case 2:
            {
                system("cls");
                printf("%s is defending the next attack!\n", chara1->name);
                chara1->state = 2;
                score++;
                getchar();
                pressEnterToContinue();
                system("cls");
                break;
            }
            case 3:
            {
                system("cls");
                printf("----------Your current status----------\n");
                printChara(chara1);
                printf("\n------Your Weapons and Inventory-------\n");
                showWeapons(chara1);
                
                getchar();
                pressEnterToContinue();
                system("cls");
                break;
            }
            case 4:
            {
                system("cls");
                printf("------------Enemy's status------------\n");
                printChara(chara2);
                printf("\n-----Enemy's Weapon and Inventory-----\n");
                showWeapons(chara2);

                getchar();
                pressEnterToContinue();
                system("cls");
                break;
            }
        }
    } while (op == 3 || op > 5 || op < 1);
   return 0;
}

int aiPhase(Character * chara1, Character * chara2)
{
    int op;
    printf("-----Enemy turn!-----\n");
    pressEnterToContinue();
    system("cls");
    int chance;
    if(chara1->currentHP >= chara1->currentHP/2) op = 1;
    else
    {
        if(randIntLimits(1,100) >= 50) op = 1;
        else op = 2;
    }
    switch(op)
    {
        case 1:
        {
            printf("%s attacks!\n", chara1->name);
            chara1->state = 1;
            if(attack(chara1, chara2) == 1) return 1;
            break;
        }
        case 2:
        {
            printf("%s is defending the next attack!\n", chara1->name);
            chara1->state = 2;
            break;
        }
    }
    pressEnterToContinue();
    system("cls");
}

void saveScore(Character * chara, RunManager * runManager)
{
    highScoresFile = fopen("highscores.csv", "a");
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

void play(RunManager * runManager)
{
    system("cls");
    Character * playerChara = createChara();
    playerChara->isPlayer = 1;
    int state = 1, chara2Level, battleResults;
    runManager->turn = 0;

    system("cls");
    pressEnterToContinue();
    printf("\nYour character's (randomly generated) stats:\n");
    printChara(playerChara);
    giveWeapon(playerChara, getRanWeapon(1));
    playerChara->currentWeapon = 1;
    printf("\nYou brought a %s as a weapon!\n", playerChara->weapons[playerChara->currentWeapon]->name);

    Character * enemyChara;

    printf("\n");
    while(state != 0)
    {
        if(runManager->turn > 20) state = 4;
        switch(state)
        {
            case 1:
            {
                // Before a new combat, an Enemy has to be generated
                pressEnterToContinue();
                system("cls");
                printf("-----Before combat-----\n");
                runManager->turn = 0, runManager->room++, runManager->score++;
                if(runManager->room%11 == 0) runManager->floor++, runManager->room = 0, runManager->score+=2;

                printf("Floor [%i] Room [%i]\n", runManager->floor, runManager->room);
                pressEnterToContinue();
                system("cls");

                printf("Generating new enemy\n");
                pressEnterToContinue();
                chara2Level = fmax(1,(int)((randFloatLimits(80, 120)/100)*playerChara->level));

                enemyChara = genRanChara(chara2Level);

                //Entering the Combating state
                state = 2;
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
                    battleResults = playerPhase(playerChara, enemyChara, runManager->score, runManager);
                    if(battleResults == 2) return;
                    else if(battleResults == 1)
                    {
                        printf("The foe has been defeated!\n");
                        state = 3;
                        runManager->score+=3;
                        pressEnterToContinue();
                        continue;
                    }
                    
                    if(aiPhase(enemyChara, playerChara) == 1)
                    {
                        printf("You have been defeated...\n");
                        state = 5;
                        pressEnterToContinue();
                        continue;
                    }
                }
                else if(playerChara->stats->spd <= enemyChara->stats->spd)
                {
                    
                    if(aiPhase(enemyChara, playerChara) == 1)
                    {
                        printf("You have been defeated...\n");
                        state = 5;
                        pressEnterToContinue();
                        continue;
                    }
                    battleResults = playerPhase(playerChara, enemyChara, runManager->score, runManager);
                    if(battleResults == 2) return;
                    else if(battleResults == 1)
                    {
                        printf("The foe has been defeated!\n");
                        state = 3;
                        runManager->score+=3;
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
                printf("-----After combat-----\n");
                printf("Your current state:\n");
                printShortChara(playerChara);
                state = 1;
                runManager->turn = 0;
                break;
            }
            case 4:
            {
                system("cls");
                printf("You took so long to defeat the enemy that more enemies came and you coulnd't against all of them.\n");
                state = 5;
            }
            case 5:
            {
                system("cls");
                printf("Game over");
                pressEnterToContinue();
                saveScore(playerChara, runManager);
                state = 0;
                return;
            }

        }
    }

}

void showHighScores()
{
    highScoresFile = fopen("highscores.csv", "r");
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
}

void readWeapons()
{
    FILE * f = fopen("weapons.csv", "r");
    if(f == NULL)
    {
        printf("ERROR: The file [weapons.csv] is missing.");
        exit(1);
    }
    printf("mallocing weaponsList**\n");
    weaponsList = (WeaponListNode *) malloc (5*sizeof(WeaponListNode));
    int i;
    printf("creating the 4 lists\n");
    for(i = 0 ; i < 6 ; i++)
    {
        weaponsList[i].list = createList();
        if(weaponsList[i].list == NULL)
        {
            printf("There was an error while reading the [weapons.csv] file.");
            exit(1);
        }
        weaponsList[i].amount = 0;
    }
    char l[1024];
    i = 0;
    while(fgets(l, 1023, f) != NULL)
    {
        if(i == 0) i = 1;
        else
        {
            Weapon * newWeapon = (Weapon *) malloc (sizeof(Weapon));
            if(newWeapon == NULL)
            {
                printf("There was an error while reading the [weapons.csv] file.");
                exit(1);
            }
            strcpy(newWeapon->name, getCSVField(l, 0));
            strcpy(newWeapon->description, getCSVField(l, 1));
            newWeapon->rarity = atoi(getCSVField(l,2));
            newWeapon->maxDurability = atoi(getCSVField(l,3));
            newWeapon->currentDurability = atoi(getCSVField(l,3));
            newWeapon->effectiveness = atoi(getCSVField(l,4));
            newWeapon->damageType = atoi(getCSVField(l,5));            
            newWeapon->stats = (Stats *) malloc (sizeof(Stats));
            if(newWeapon->stats == NULL)
            {
                printf("There was an error while reading the [weapons.csv] file.");
                exit(1);
            }
            newWeapon->stats->hp = atoi(getCSVField(l,6));
            newWeapon->stats->atk = atoi(getCSVField(l,7));
            newWeapon->stats->spd = atoi(getCSVField(l,8));
            newWeapon->stats->def = atoi(getCSVField(l,9));
            newWeapon->stats->res = atoi(getCSVField(l,10));

            //printWeapon(newWeapon);
            pushBack(weaponsList[newWeapon->rarity-1].list, newWeapon);
            weaponsList[newWeapon->rarity-1].amount++;
        }
    }
    fclose(f);
}

void openRaces()
{
    racesFile = (RacesFile *) malloc (sizeof(RacesFile));
    racesFile->file = fopen("races.csv", "r");
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

void openAllFiles()
{
    readWeapons();
    printf("[weapons.csv] read successfully\n");
    openRaces();
    printf("[races.csv] read successfully\n");
    printf("[highscores.csv] read successfully\n");
}

int main()
{
    srand(time(NULL));
    int op = 0, c;
    openAllFiles();
    printf("------------------Start------------------\n");

    while(op != 4)
    {
        pressEnterToContinue();
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
                if(c == 1)
                {
                RunManager * runManager = (RunManager*) malloc (sizeof(RunManager));
                runManager->room = 0;
                runManager->floor = 1;
                runManager->score = 0;
                play(runManager);
                }
                break;
            }
            case 2:
            {
                system("cls");
                printf("Not yet available in this version.\n");
                break;
            }
            case 3:
            {
                system("cls");
                showHighScores();
                break;
            }
            case 4:
            {
                printf("Exiting the game...");
                break;
            }
        }
    }
    return 0;
}
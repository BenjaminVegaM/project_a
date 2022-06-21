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

//Global Variable
WeaponListNode * weaponsList;

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

    int open_mark = 0;
    char* ret=(char*) malloc (100*sizeof(char));
    int ini_i=0, i=0;
    int j=0;
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
    printf("...\n");
    char enter = 0;
    while (enter != '\r' && enter != '\n')
    {
        enter = getchar();
    }
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
    FILE * file = fopen("races.csv", "r");
    if(file == NULL)
    {
        printf("ERROR: The file [races.csv] is missing.");
        exit(1);
    }

    if(fgets(l, 1024, file) == NULL || getCSVField(l, nInt) == NULL)
    {
        fclose(file);
        return "Unkown";
    }
    fclose(file);
    return getCSVField(l, nInt);
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

int getRanWpnRarity()
{
    //dependiendo del nivel tiene mas o menos chance de cada rareza
    int n = randIntLimits(0,5);
    return n;
}

Weapon * getRanWeapon(int rarity)
{
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
    while(b < 3 && chara->weapons[b] != NULL)
    {
        b++;
    }
    chara->weapons[b] = weapon;
}

void chooseNextAvailableWeapon(Character * chara)
{
    int i = 3;
    while(chara->weapons[i] == NULL) i--;
    chara->currentWeapon = i;
}

void readWeapons()
{
    printf("Opening file\n");
    FILE * f = fopen("weapons.csv", "r");
    if(f == NULL)
    {
        printf("ERROR: The file [weapons.csv] is missing.");
        exit(1);
    }
    printf("mallocing List**\n");
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
    char l[4];
    FILE * file = fopen("races.csv", "r");
    if(file == NULL)
    {
        printf("ERROR: The file [races.csv] is missing.");
        exit(1);
    }
    if(fgets(l, 3, file) == NULL) strcpy(l, "0");
    fclose(file);
    int race = randIntLimits(1, atoi(getCSVField(l, 0)));
    Character * newChara = generateChara(genRanCharStats(level), level, (char*)getRaceString(race), race);
    newChara->experience = randIntLimits(0, newChara->expToLvlUp-1);
    giveWeapon(newChara, getRanWeapon(getRanWpnRarity()));
    chooseNextAvailableWeapon(newChara);
    return newChara;
}

Character * createChara()
{
    printf("What is their name?\n");
    char name[25];
    getchar();
    gets(name);

    Stats * stats = genRanCharStats(3);

    Character * newChar = generateChara(stats, 1, name, 1);
    return newChar;
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
            if(chara->currentWeapon == 0) printf("Now they are unarmed!");
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
    printf("%s +%i exp\n", chara->name, exp);
    printf("Exp [%i/%i]\n", chara->experience, chara->expToLvlUp);
    if(chara->experience > chara->expToLvlUp)
    {
        chara->experience-=chara->expToLvlUp;
        levelUp(chara);
    }
}

int damageCalc(Character * attacker, Character * defender)
{
    int dmgReduction, damageDealt;
    switch (attacker->weapons[attacker->currentWeapon]->damageType)
    {
        case 0: dmgReduction = 0;
        case 1: dmgReduction = defender->stats->def; break;
        case 2: dmgReduction = defender->stats->res; break;
    }
    damageDealt = fmax(1, attacker->stats->atk + attacker->weapons[attacker->currentWeapon]->stats->atk - dmgReduction);
    if(defender->state == 2)
    {
        damageDealt = damageDealt/2;
        if(randIntLimits(1,10) == 1)
        {
            damageDealt = 0;
            printf("%s completely blocked the attack!\n", defender->name);
        }
    }
    printf("Damage dealt = %i\n", damageDealt);
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
        printf("%s defended the attack and is not defending anymore.\n", defender->name);
        giveExperience(defender, 1);
    }
    printf("%s's HP [%i/%i]\n",defender->name, defender->currentHP, defender->stats->hp);
    pressEnterToContinue();
    lowerWeaponDurability(attacker);
    attacker->state = 0;
    if(defender->currentHP == 0)
    {
        printf("%s has been defeated!\n", defender->name);
        giveExperience(attacker, defender->level*6);
        pressEnterToContinue();
        return 1;
    }
    return 0;
}

int playerPhase(Character * chara1, Character * chara2, int score)
{
    int op;
    printf("-----Your turn!-----\n");
    do
    {
        printf("1. Attack\n");
        printf("2. Defend\n");
        printf("3. Show Player Info (Does not count as an action)\n");
        printf("4. Check enemy info\n");
        scanf("%i", &op);
        switch(op)
        {
            case 1:
            {
                if(chara1->currentWeapon != 0)
                {
                    printf("Choose a weapon to attack with\n");
                    showWeapons(chara1);
                    while(1)
                    {
                        scanf("%i", chara1->currentWeapon);
                        if(chara1->weapons[chara1->currentWeapon] != NULL) break;
                        else printf("Not valid\n");
                    }
                }
                printf("%s attacks!\n", chara1->name);
                chara1->state = 1;
                score+=2;
                if(attack(chara1, chara2) == 1) return 1;
                pressEnterToContinue();
                break;
            }
            case 2:
            {
                printf("%s is defending the next attack!\n", chara1->name);
                chara1->state = 2;
                score++;
                pressEnterToContinue();
                break;
            }
            case 3:
            {
                printf("----------Your current status----------\n");
                printChara(chara1);
                printf("------Your Weapons and Inventory-------\n");
                showWeapons(chara1);
                
                pressEnterToContinue();
                break;
            }
            case 4:
            {
                printf("------------Enemy's status------------\n");
                printChara(chara2);
                printf("-----Enemy's Weapon and Inventory-----\n");
                showWeapons(chara2);

                pressEnterToContinue();
                break;
            }
        }
    } while (op == 3);
   return 0;
}

int aiPhase(Character * chara1, Character * chara2)
{
    int op;
    printf("-----Enemy turn!-----\n");
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
            printf("Enemy current weapon = %i\n", chara1->currentWeapon);
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
}

void saveScore(Character * chara, int score)
{
    printf("Your score with %s was [%i]\n", chara->name, score);
}

void play()
{
    Character * playerChara = createChara();
    playerChara->isPlayer = 1;
    int state = 1, turn, room = 0, floor = 1, score = 0, chara2Level;

    printf("\nYour character's (randomly generated) stats:\n");
    printChara(playerChara);
    printf("Giving you a weapon.\n");
    giveWeapon(playerChara, getRanWeapon(1));
    playerChara->currentWeapon = 1;
    printf("You got a %s!\n", playerChara->weapons[playerChara->currentWeapon]->name);

    Character * enemyChara;

    printf("\n");
    while(state != 0)
    {
        switch(state)
        {
            case 1:
            {
                // Before a new combat, an Enemy has to be generated
                pressEnterToContinue();
                printf("-----Before combat-----\n");
                turn = 0, room++, score++;
                if(room%11 == 0) floor++, room = 0, score+=2;

                printf("Floor [%i] Room [%i]\n", floor, room);
                pressEnterToContinue();

                printf("Generating new enemy\n");
                pressEnterToContinue();
                chara2Level = fmax(1,(int)((randFloatLimits(80, 120)/100)*playerChara->level));

                enemyChara = genRanChara(chara2Level);
                printChara(enemyChara);

                //Entering the Combating state
                state = 2;
                break;
            }
            case 2:
            {
                turn++;
                printf("-----Turn %i-----\n", turn);
                pressEnterToContinue();
                if(playerChara->stats->spd > enemyChara->stats->spd)
                {
                    if(playerPhase(playerChara, enemyChara, score) == 1)
                    {
                        printf("The foe has been defeated!\n");
                        state = 3;
                        score+=3;
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
                    if(playerPhase(playerChara, enemyChara, score) == 1)
                    {
                        printf("The foe has been defeated!\n");
                        state = 3;
                        score+=3;
                        pressEnterToContinue();
                        continue;
                    }
                }
                break;
            }
            case 3:
            {
                printf("-----After combat-----\n");
                printf("Your current state:\n");
                printChara(playerChara);
                state = 1;
                if(turn > 20) state = 4;
                pressEnterToContinue();
                break;
            }
            case 4:
            {
                printf("You took so long to defeat the enemy that more enemies came and you coulnd't against all of them.\n");
                state = 5;
            }
            case 5:
            {
                printf("Game over");
                pressEnterToContinue();
                saveScore(playerChara, score);
                state = 0;
                return;
            }

        }
    }

}

void showHighScores()
{
    char line[25];
    FILE * file = fopen("highscores.csv", "r");
    if(file == NULL)
    {
        printf("ERROR: The file [highscores.csv] is missing.\n");
        exit(1);
    }
    
    if(fgets(line, 1023, file) == NULL)
    {
        printf("There are no HighScores yet, go and be the first!\n");
        return;
    }
    
    printf("-----------------HIGHSCORES-----------------\n");

    do{
        printf("%-24s %19i\n", getCSVField(line, 0), atoi(getCSVField(line, 1)));
    }while(fgets(line, 1023, file) != NULL);
    fclose(file);
    pressEnterToContinue();
}

int main()
{
    srand(time(NULL));
    int op = 0;
    readWeapons();
    printf("weapons.csv read successfully\n");
    printf("------------------Start------------------\n");

    while(op != 4)
    {
        pressEnterToContinue();
        printf("\n--------------------MENU--------------------\n");
        printf("1. New run\n");
        printf("2. Continue\n");
        printf("3. Highscores\n");
        printf("4. Exit\n");
        printf("--------------------------------------------\n");
        printf("Input an option:\n");
        scanf("%i", &op);
        switch(op)
        {
            case 1:
            {
                play();
                break;
            }
            case 2:
            {
                printf("Not yet available in this version.\n");
                break;
            }
            case 3:
            {
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
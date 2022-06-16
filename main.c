#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

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
    char name[25];
    int race; //a number that represents the race
    int level;
    int state;
    int currentHP;
    Stats * stats;
} Character;

typedef struct
{
    char name[40];
    char description[500];
    int effectiveness; //same number as race
    //increase in char stats
    Stats * stats;
} Weapon;

//generates a random Integer number within a range
int randIntLimits(int lower, int upper) {
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

//generates a random Float number within a range
float randFloatLimits(int lower, int upper) {
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
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
Stats * genRanCharaStats(int level)
{
    float randNum;
    Stats * stats = (Stats *) malloc (sizeof (Stats));

    randNum = randFloatLimits(80, 120)/100;
    stats->hp = level * randNum + 5;

    randNum = randFloatLimits(80, 120)/100;
    stats->atk = level * randNum;

    randNum = randFloatLimits(80, 120)/100;
    stats->spd = level * randNum;

    randNum = randFloatLimits(80, 120)/100;
    stats->def = level * randNum;

    randNum = randFloatLimits(80, 120)/100;
    stats->res = level * randNum;

    return stats;
}

const char * getRaceString(int raceInt)
{
    switch(raceInt)
    {
        case 0:
        {
            return "Unknown";
        }
        case 1:
        {
            return "Human";
        }
        case 2:
        {
            return "Elf";
        }
        case 3:
        {
            return "Dwarf";
        }
        case 4:
        {
            return "Goblin";
        }
        case 5:
        {
            return "Undead";
        }
        case 6:
        {
            return "Dragon";
        }
    }
}

/*
const char * getRaceString(int raceInt)
{
    char * raceString = malloc(25);
    switch(raceInt)
    {
        case 0:
        {
            strcpy(raceString, "Unknown");
        }
        case 1:
        {
            strcpy(raceString, "Human");
        }
        case 2:
        {
            strcpy(raceString, "Elf");
        }
        case 3:
        {
            strcpy(raceString, "Dwarf");
        }
        case 4:
        {
            strcpy(raceString, "Goblin");
        }
        case 5:
        {
            strcpy(raceString, "Undead");
        }
        case 6:
        {
            strcpy(raceString, "Dragon");
        }
    }
    return raceString;
}
*/

// Generates a character with the stats
Character * generateChara(Stats * stats, int level, char * name, int race)
{
    Character * newChar = (Character *) malloc (sizeof(Character));
    strcpy(newChar->name, name);
    newChar->race = race;
    newChar->level = level;
    newChar->state = 0;
    newChar->stats = stats;
    newChar->currentHP = stats->hp;
    return newChar;
}

Character * genRanChara(int level)
{
    int race = randIntLimits(0, 6);
    Character * newChar = generateChara(genRanCharaStats(level), level, (char*)getRaceString(race), race);
    return newChar;
}

Character * createChara()
{
    printf("What is their name?\n");
    char name[25];
    getchar();
    gets(name);

    Stats * stats = genRanCharaStats(10);

    Character * newChar = generateChara(stats, 1, name, 1);
    return newChar;
}

void printChara(Character * chara)
{
    printf("Name  [%s]\n", chara->name);
    printf("Level [%i]\n", chara->level);
    printf("Race  [%s]\n", getRaceString(chara->race));
    printf("Stats:\n");
    printf("    HP  [%i/%i]\n", chara->currentHP, chara->stats->hp);
    printf("    Atk [%i]\n", chara->stats->atk);
    printf("    Spd [%i]\n", chara->stats->spd);
    printf("    Def [%i]\n", chara->stats->def);
    printf("    Res [%i]\n", chara->stats->res);
}

void levelUp(Character * chara)
{
    printf("-----LEVEL UP!-----\n");
    chara->level++;

    int chance = randIntLimits(0, 100);
    if(chance <= 65)
    {
        chara->stats->hp++;
        chara->currentHP++;
        printf("HP +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 60)
    {
        chara->stats->atk++;
        printf("Atk +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 40)
    {
        chara->stats->spd++;
        printf("Spd +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 40)
    {
        chara->stats->def++;
        printf("Def +1\n");
    }

    chance = randIntLimits(0, 100);
    if(chance <= 50)
    {
        chara->stats->res++;
        printf("Res +1\n");
    }
    printf("-------------------\n");
}

int dealDamage(Character * attacker, Character * defender)
{
    int damageDealt;
    damageDealt = attacker->stats->atk - defender->stats->def;
    if(defender->state == 2) damageDealt = damageDealt/2;

    if(damageDealt < 1) damageDealt = 1;

    printf("Damage dealt = %i\n", damageDealt);
    return damageDealt;
}

//Returns 0 if the enemy is alive, and 1 if the enemy died because of the attack
int attack(Character * attacker, Character * defender)
{
    defender->currentHP -= dealDamage(attacker, defender);
    if(defender->state == 2)
    {
        defender->state = 0;
        pressEnterToContinue();
        printf("%s defended the attack and is not defending anymore.\n", defender->name);
    }
    if(defender->currentHP < 0) defender->currentHP = 0;
    pressEnterToContinue();
    printf("%s's HP [%i/%i]\n",defender->name, defender->currentHP, defender->stats->hp);
    pressEnterToContinue();
    if(defender->currentHP == 0)
    {
        printf("%s has been defeated!\n", defender->name);
        levelUp(attacker);
        pressEnterToContinue();
        return 1;
    }
    return 0;
}

int playerPhase(Character * chara1, Character * chara2)
{
    int op;
    printf("-----Your turn!-----\n");
    do
    {
        printf("1. Attack\n2. Defend\n3. Show current status\n");
        scanf("%i", &op);
        switch(op)
        {
            case 1:
            {
                printf("%s attacks!\n", chara1->name);
                chara1->state = 1;
                if(attack(chara1, chara2) == 1) return 1;
            }
            case 2:
            {
                printf("%s is defending the next attack!\n", chara1->name);
                chara1->state = 2;
            }
            case 3:
            printf("Your current status:\n");
            printChara(chara1);
        }
        pressEnterToContinue();
    } while (op == 3);
    
}

int aiPhase(Character * chara1, Character * chara2)
{
    int op;
    printf("-----Enemy turn!-----\n");
    int chance;
    if(chara1->currentHP >= chara1->currentHP/2)
    {
        op = 1;
    }
    else
    {
        if(randIntLimits(1,100) >= 50)
        {
            op = 1;
        }
        else
        {
            op = 2;
        }
    }
    switch(op)
    {
        case 1:
        {
            printf("%s attacks!\n", chara1->name);
            chara1->state = 1;
            if(attack(chara1, chara2) == 1) return 1;
        }
        case 2:
        {
            printf("%s is defending the next attack!\n", chara1->name);
            chara1->state = 2;
        }
    }
    pressEnterToContinue();
}

void play()
{
    int state = 1;
    int turn;
    Character * chara1 = createChara();
    printf("\nYour character's (randomly generated) stats:\n");
    printChara(chara1);
    Character * chara2;

    printf("\n");

    while(state != 0)
    {
        switch(state)
        {
            case 1:
            {
                pressEnterToContinue();
                printf("-----Before combat-----\n");
                turn = 0;
                printf("Generating new enemy\n");
                pressEnterToContinue();
                chara2 = genRanChara((int)((randFloatLimits(80, 120)/100)*chara1->level));
                printChara(chara2);
                state = 2;
                break;
            }
            case 2:
            {
                turn++;
                printf("-----Turn %i-----\n", turn);
                pressEnterToContinue();
                if(chara1->stats->spd > chara2->stats->spd)
                {
                    if(playerPhase(chara1, chara2) == 1)
                    {
                        state = 3;
                        printf("The foe has been defeated!\n");
                        pressEnterToContinue();
                        continue;
                    }
                    if(aiPhase(chara2, chara1) == 1)
                    {
                        state = 5;
                        printf("You have been defeated...\n");
                        pressEnterToContinue();
                        continue;
                    }
                }
                else if(chara1->stats->spd <= chara2->stats->spd)
                {
                    if(aiPhase(chara2, chara1) == 1)
                    {
                        state = 5;
                        printf("You have been defeated...\n");
                        pressEnterToContinue();
                        continue;
                    }
                    if(playerPhase(chara1, chara2) == 1)
                    {
                        state = 3;
                        printf("The foe has been defeated!\n");
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
                printChara(chara1);
                state = 1;
                if(turn > 20) state = 0;
                pressEnterToContinue();
                break;
            }
            case 4:
            {
                state = 0;
                break;
            }
            case 5:
            {
                printf("Game over");
                pressEnterToContinue();
                return;
            }

        }
    }

}

int main()
{
    srand(time(NULL));
    int op = 0;
    printf("------------------Start------------------\n");

    while(op != 3)
    {
        pressEnterToContinue();
        printf("\n--------------------MENU--------------------\n");
        printf("1. Play\n");
        printf("2. Highscores\n");
        printf("3. Exit\n");
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
                printf("Not available in this version.\n");
                break;
            }
            case 3:
            {
                printf("Exiting...");
                break;
            }
        }
    }
    return 0;
}
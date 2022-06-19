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
    int isPlayer; //1 Player, 0 otherwise
    char name[25];
    int race; //a number that represents the race
    int level;
    int experience;
    int expToLvlUp;
    int state;
    int currentHP;
    Stats * stats;
    List * inventory;

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
Stats * genRanStats(int level)
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

//Función para tomar un string de una coordenada específica de un archivo .csv
const char *get_csv_field (char * tmp, int k) {
    int open_mark = 0;
    char* ret=(char*) malloc (100*sizeof(char));
    int ini_i=0, i=0;
    int j=0;
    while(tmp[i+1]!='\0' && tmp[i+1]!='\n'){

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
    newChar->experience = 0;
    newChar->expToLvlUp = level*5;
    newChar->state = 0;
    newChar->stats = stats;
    newChar->currentHP = stats->hp;
    newChar->inventory = createList();
    newChar->isPlayer = 0;
    return newChar;
}

void generateRandomWeaponInventory(Character * chara)
{
    int i;
    for(i = 0 ; i < 3 ; i++)
    {
        printf("Generando arma %i\n", i+1);
        Weapon * newWeapon = (Weapon*) malloc (sizeof(Weapon));
        strcpy(newWeapon->description, "This is the description");
        newWeapon->stats = genRanStats(5);
        newWeapon->stats->hp*=10;
        newWeapon->effectiveness = randIntLimits(0,3);
        strcpy(newWeapon->name, "Super Cool Weapon");

        pushBack(chara->inventory, newWeapon);
        printf("Arma %i obtenida\n", i+1);
    }
}

void printWeapon(Weapon * weapon)
{
    printf("Name: [%s]\n", weapon->name);
    printf("Description: [%s]\n", weapon->description);
}

void showInventory(Character * chara)
{
    if(chara->inventory == NULL)
    {
        printf("The inventory is empty!\n");
        return;
    }

    Weapon * auxWpn = firstList(chara->inventory);

    if(chara->inventory == NULL)
    {
        printf("The inventory is empty!\n");
        return;
    }

    int i = 0;
    while(auxWpn != NULL && i < 3)
    {
        i++;
        printWeapon(auxWpn);
    }
}

Character * genRanChara(int level)
{
    int race = randIntLimits(0, 6);
    Character * newChar = generateChara(genRanStats(level), level, (char*)getRaceString(race), race);
    return newChar;
}

Character * createChara()
{
    printf("What is their name?\n");
    char name[25];
    getchar();
    gets(name);

    Stats * stats = genRanStats(3);

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

    chara->expToLvlUp+=5;
}

void giveExperience(Character * chara, int exp)
{
    chara->experience += exp;
    if(chara->experience > chara->expToLvlUp)
    {
        chara->experience-=chara->expToLvlUp;
        levelUp(chara);
    }
}

int dealDamage(Character * attacker, Character * defender)
{
    int damageDealt = fmax(1, attacker->stats->atk - defender->stats->def);
    if(defender->state == 2) damageDealt = fmax(1, damageDealt/2);

    printf("Damage dealt = %i\n", damageDealt);
    return damageDealt;
}

//Returns 0 if the enemy is alive, and 1 if the enemy died because of the attack
int attack(Character * attacker, Character * defender)
{
    defender->currentHP = fmax(0, defender->currentHP - dealDamage(attacker, defender));
    pressEnterToContinue();

    if(defender->state == 2)
    {
        defender->state = 0;
        printf("%s defended the attack and is not defending anymore.\n", defender->name);
    }
    printf("%s's HP [%i/%i]\n",defender->name, defender->currentHP, defender->stats->hp);
    pressEnterToContinue();
    attacker->state = 0;
    if(defender->currentHP == 0)
    {
        printf("%s has been defeated!\n", defender->name);
        levelUp(attacker);
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
        printf("3. Show current status\n");
        scanf("%i", &op);
        switch(op)
        {
            case 1:
            {
                printf("%s attacks!\n", chara1->name);
                chara1->state = 1;
                score+=2;
                if(attack(chara1, chara2) == 1) return 1;
                break;
            }
            case 2:
            {
                printf("%s is defending the next attack!\n", chara1->name);
                chara1->state = 2;
                score++;
                break;
            }
            case 3:
            {
                printf("Your current status:\n");
                printChara(chara1);
                break;
            }
        }
        pressEnterToContinue();
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
                chara2Level = (int)((randFloatLimits(80, 120)/100)*playerChara->level);
                
                //In case the level is 0
                if(chara2Level == 0) chara2Level = 1;

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
        printf("There was an error loading the highscores, the file highscores.csv may be damaged.\n");
        exit(1);
    }
    
    if(fgets(line, 1023, file) == NULL)
    {
        printf("There are no HighScores yet, go and be the first!\n");
        return;
    }
    
    printf("-----------------HIGHSCORES-----------------\n");

    do{
        printf("%-24s %19i\n", get_csv_field(line, 0), atoi(get_csv_field(line, 1)));
    }while(fgets(line, 1023, file) != NULL);
    fclose(file);
    pressEnterToContinue();
}

int main()
{
    srand(time(NULL));
    int op = 0;
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
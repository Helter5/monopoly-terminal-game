#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "monopoly.h" // NEMENIT !!!

int numberOfPlayers;
int turn = 0;
int gameOver = 0;
int crashedPlayer = 0;
int s = 0, p = 0, g = 0;

void spawnPlayers(PLAYER players[]){
    int cash;
    cash = (numberOfPlayers == 2) ? 20 : (numberOfPlayers == 3) ? 18 : 16;
    for(int i=0; i<numberOfPlayers;i++){
        players[i].number = i+1;
        players[i].space_number = 0;
        players[i].cash = cash;
        players[i].num_jail_pass = 0;
        players[i].is_in_jail = 0;
        players[i].num_properties = 0;
    }
}

void movePlayer(PLAYER players[], int playerID, int step){
    int temp;
    if(players[playerID].space_number+step<NUM_SPACES){
        players[playerID].space_number += step;
    } else {
        temp = (players[playerID].space_number+step)-NUM_SPACES;
        players[playerID].space_number = temp;
        players[playerID].cash += 2;
    }
}

int getPropertyOwner(PLAYER players[], int playerID){
    int spaceNumber = players[playerID].space_number;
    PROPERTY *property = game_board[spaceNumber].property;

    for(int i=0; i<numberOfPlayers; i++){
        for(int j=0; j<players[i].num_properties; j++){
            if(players[i].owned_properties[j] == property){
                return i;
            }
        }
    }
    return -1;
}

int hasMonopol(PLAYER players[], int playerID, int ownerID){
    int spaceNumber = players[playerID].space_number;
    PROPERTY *steppedProperty = game_board[spaceNumber].property;

    if(ownerID == -1) {
        return 0;
    }

    int colorCount = 0;
    for(int i=0; i<players[ownerID].num_properties; i++){
        if(players[ownerID].owned_properties[i]->color == steppedProperty->color){
            colorCount++;
        }
    }

    return (colorCount == 2) ? 1 : 0;
}

void handleSpace(PLAYER players[], int playerID){
    if(players[playerID].is_in_jail == 1) {
        if (players[playerID].cash >= 1) {
            players[playerID].is_in_jail = 0;
            players[playerID].cash -= 1;
        } else {
            gameOver = 1;
            crashedPlayer = playerID;
        }
    }

    SPACE currentSpace = game_board[players[playerID].space_number];

    if(currentSpace.type == Property){
        PROPERTY property = *currentSpace.property;
        int ownerID = getPropertyOwner(players, playerID);
        if(ownerID == -1) {
            if (players[playerID].cash >= property.price) {
                players[playerID].cash -= property.price;
                players[playerID].owned_properties[players[playerID].num_properties] = currentSpace.property;
                players[playerID].num_properties++;
            } else {
                gameOver = 1;
                crashedPlayer = playerID;
            }
        } else if (ownerID != playerID){
            int rent = property.price;
            if(hasMonopol(players, playerID, ownerID) == 1){
                rent *= 2;
            }
            if(players[playerID].cash >= rent){
                players[playerID].cash -= rent;
                players[ownerID].cash += rent;
            } else {
                gameOver = 1;
                crashedPlayer = playerID;
            }
        }
    } else if(currentSpace.type == Jail_pass){
        players[playerID].num_jail_pass++;
    } else if(currentSpace.type == Go_to_jail){
        if(players[playerID].num_jail_pass > 0){
            players[playerID].num_jail_pass--;
        } else {
            players[playerID].space_number = 6;
            players[playerID].is_in_jail = 1;
        }
    }
}

int findPropertyLocation(PROPERTY* target_property){
    for(int i=0; i<NUM_SPACES; i++){
        if(game_board[i].type == Property && game_board[i].property == target_property){
            return i+1;
        }
    }
    return -1;
}

void printPlayers(PLAYER players[], int choice){
    if(choice == 0) {
        printf("Players:\n");
        for (int i = 0; i < numberOfPlayers; i++) {
            printf("%d. S: %d, C: %d, JP: %d, IJ: %s\n", players[i].number, players[i].space_number+1, players[i].cash,
                   players[i].num_jail_pass, (players[i].is_in_jail == 1) ? "yes" : "no");
        }
    } else if(choice == 1) {
        printf("Players:\n");
        for(int i=0; i<numberOfPlayers; i++){
            printf("%d. S: %d, C: %d, JP: %d, IJ: %s\n", players[i].number, players[i].space_number+1, players[i].cash,
                   players[i].num_jail_pass, (players[i].is_in_jail == 1) ? "yes" : "no");
            for(int j=0; j<players[i].num_properties; j++){
                PROPERTY *property = players[i].owned_properties[j];
                printf("      %-16s %-2d %-10s S%d\n", property->name, property->price, property_colors[property->color],
                       findPropertyLocation(property));
            }
        }
    }
    //printf("\n");
}

char* getWinner(PLAYER players[]){
    int maxCash = -1;
    int winnerID = -1;
    int isDraw = 0;

    for(int i=0; i<numberOfPlayers; i++){
        if(i == crashedPlayer) continue;

        if(players[i].cash > maxCash){
            maxCash = players[i].cash;
            winnerID = i;
            isDraw = 0;
        } else if(players[i].cash == maxCash){
            isDraw = 1;
        }
    }

    if(isDraw){
        int maxPropertyValue = -1;
        int isPropertyDraw = 0;
        for(int i=0;i<numberOfPlayers;i++){
            if(i == crashedPlayer) continue;

            if(players[i].cash == maxCash){
                int totalPropertyValue = 0;
                for(int j=0; j<players[i].num_properties; j++){
                    totalPropertyValue += players[i].owned_properties[j]->price;
                }
                if(totalPropertyValue > maxPropertyValue){
                    maxPropertyValue = totalPropertyValue;
                    winnerID = i;
                    isPropertyDraw = 0;
                } else if(totalPropertyValue == maxPropertyValue){
                    isPropertyDraw = 1;
                }
            }
        }
        if(isPropertyDraw){
            return "?";
        }
    }

    char* winner = malloc(3 * sizeof(char));
    sprintf(winner, "P%d", winnerID+1);
    return winner;
}

void startGameboard(PLAYER players[]){
    for (int i = 0; i < NUM_SPACES; i++) {
        if (game_board[i].type == Property) {
            printf("%d. %-20s %2d %-10s\n", i + 1, game_board[i].property->name, game_board[i].property->price,
                   property_colors[game_board[i].property->color]);
        } else {
            printf("%d. %-20s\n", i + 1, space_types[game_board[i].type]);
        }
    }
    printf("WINNER: -");
}

void endGameboard(PLAYER players[], int printWinner) {
    for (int i = 0; i < NUM_SPACES; i++) {
        if (game_board[i].type == Property) {
            printf("%d. %-20s %2d %-10s", i + 1, game_board[i].property->name, game_board[i].property->price,
                   property_colors[game_board[i].property->color]);
            for (int j = 0; j < numberOfPlayers; j++) {
                PLAYER player = players[j];
                int count = 0;
                int ownsCurrent = 0;
                for (int k = 0; k < player.num_properties; k++) {
                    if (player.owned_properties[k]->color == game_board[i].property->color) {
                        count++;
                    }
                    if (player.owned_properties[k] == game_board[i].property) {
                        ownsCurrent = 1;
                    }
                }
                if (ownsCurrent) {
                    if (count >= 2) {
                        printf(" P%-1d yes", player.number);
                    } else {
                        printf(" P%-1d no", player.number);
                    }
                }
            }
            printf("\n");
        } else {
            printf("%d. %-20s\n", i + 1, space_types[game_board[i].type]);
        }
    }
    if(printWinner == 1) {
        char *winner = getWinner(players);
        printf("WINNER: %s\n", winner);
    } else if(printWinner == 2) {
        printf("WINNER: -\n");
    }
}

void printGameboard(PLAYER players[], int choice){
    printf("Game board:\n");
    if(choice == 0) {
        startGameboard(players);
    }else if(choice == 1) {
        endGameboard(players, 1);
    } else if(choice == 2){
        endGameboard(players, 0);
    } else if(choice == 3){
        endGameboard(players, 2);
    }
}

void gameLoop(PLAYER players[]){
    while(gameOver == 0) {
        for (int playerID = 0; playerID < numberOfPlayers; playerID++) {
            int step;
            if (scanf("%d", &step) != 1) {
                gameOver = 1;
                break;
            }

            turn++;
            movePlayer(players, playerID, step);
            handleSpace(players, playerID);

            printf("\nR: %d\n", step);
            printf("Turn: %d\n", turn);
            printf("Player on turn: P%d\n", playerID+1);
            printf("\n");

            if(s == 1 && gameOver == 0){
                printGameboard(players, 2);
            }

            if(p == 1 && gameOver == 0){
                printPlayers(players, 1);
            }

            if(g == 1 && gameOver == 0){
                printPlayers(players, 1);
                printGameboard(players, 3);
            }
        }
    }
}



int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "n:spg")) != -1) {
        switch (opt) {
            case 'n':
                numberOfPlayers = atoi(optarg);
                break;
            case 's':
                s = 1;
                break;
            case 'p':
                p = 1;
                break;
            case 'g':
                g = 1;
                break;
        }
    }

    PLAYER players[numberOfPlayers];

    spawnPlayers(players);
    printPlayers(players, 0);
    printGameboard(players, 0);
    printf("\n");
    gameLoop(players);
    printPlayers(players, 1);
    printGameboard(players, 1);
    return 0;
}
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>

#define BOARD_SIZE 9
#define THRONE 'T'
#define CORNER 'C'
#define EMPTY '.'
#define KING 'K'
#define DEFENDER 'D'
#define ATTACKER 'A'

struct client_pair {
    int client1;
    int client2;
};

bool confirm_receipt(int ClientFD, const char* name) {
    char buff[100];
    bzero(buff, sizeof(buff));

    int n = read(ClientFD, buff, sizeof(buff));
    if (n <= 0) {
        printf("%s rozłączony\n", name);
        return false;
    }

    if (strcmp(buff, "ok") != 0) {
        printf("Błąd potwierdzenia od %s\n", name);
        return false;
    }
    return true; 
}

bool send_board(char board[BOARD_SIZE][BOARD_SIZE], int client1, int client2) {
    //Zamiana tablicy 2d na 1d
    char board1D[BOARD_SIZE * BOARD_SIZE];
    bzero(board1D, sizeof(board1D));
    for (int i = 0; i < BOARD_SIZE; i++) { 
        for (int j = 0; j < BOARD_SIZE; j++) {
            board1D[i * BOARD_SIZE + j] = board[i][j];
        }
    }
    char buff[100];
    bzero(buff, sizeof(buff));
    strcpy(buff, "board ");
    strcat(buff, board1D); //doklejenie do buff tablicy 1-wymiarowej
    if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
        printf("Utracono połączenie z graczem 1\n");
        return false;
    }
    if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
        printf("Utracono połączenie z graczem 1\n");
        return false;
    }
    return true; 
}

// 0 - atakujacy, 1 - obronca
bool move_validation(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int new_x, int new_y, int turn) {
    if(turn == 0 && board[x][y] != ATTACKER){
        return false;
    }
    if(turn == 1 && (board[x][y] != DEFENDER && board[x][y] != KING)){
        return false;
    }
    if(x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE ||
        new_x < 0 || new_x >= BOARD_SIZE || new_y < 0 || new_y >= BOARD_SIZE) {
        return false;
    }
    if(x != new_x && y != new_y) { // Ruch musi być w linii prostej
        return false; 
    }
    if(board[x][y] == KING && board[new_x][new_y] != EMPTY && 
        board[new_x][new_y] != THRONE && board[new_x][new_y] != CORNER) { // Ruch królem
        return false;
    }
    if(board[x][y] != KING && board[new_x][new_y] != EMPTY) { // Ruch pionem
        return false;
    }
    // Sprawdzenie, czy na drodze nie ma pionków
    int direction;
    if (x == new_x) {
        if(new_y > y){
            direction = 1;
        }else{
            direction = -1;
        }
        for (int i = y + direction; i != new_y; i += direction) {
            if (board[x][i] != EMPTY && board[x][i] != THRONE){ 
                return false; // Jeśli pole jest zajęte, ruch jest niemożliwy
            }
        }
    }
    else{
        if(new_x > x){
            direction = 1;
        }else{
            direction = -1;
        }
        for (int i = x + direction; i != new_x; i += direction) {
            if (board[i][y] != EMPTY && board[i][y] != THRONE) {
                return false; // Jeśli pole jest zajęte, ruch jest niemożliwy
            }
        }
    }
    return true;
}

void make_move(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int new_x, int new_y) {
    board[new_x][new_y] = board[x][y];
    if( x == 4 && y == 4){
        board[x][y] = THRONE;
    }else{
        board[x][y] = EMPTY;
    }
}

// 0 - atakujacy, 1 - obronca
void battle(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int turn){
    bool KING_THRONE = false;
    if(turn == 0){
        if(board[4][4] == KING || board[4][3] == KING || board[4][5] == KING || board[3][4] == KING || board[5][4] == KING){
            KING_THRONE = true;
        }
        if(x < BOARD_SIZE - 2 &&  // bicie opponenta, ktory jest z prawej
            (board[x+1][y] == DEFENDER || board[x+1][y] == KING) &&
            (board[x+2][y] == ATTACKER || board[x+2][y] == CORNER || board[x+2][y] == THRONE)){
                if(!(board[x+1][y] == KING) || !KING_THRONE){
                    board[x+1][y] = EMPTY;
                }
        }
        if(x > 1 &&  // bicie opponenta, ktory jest z lewej
            (board[x-1][y] == DEFENDER || board[x-1][y] == KING) &&
            (board[x-2][y] == ATTACKER || board[x-2][y] == CORNER || board[x-2][y] == THRONE)){
                 if(!(board[x-1][y] == KING) || !KING_THRONE){
                    board[x-1][y] = EMPTY;
                }
        }
        if(y < BOARD_SIZE - 2 &&  // bicie opponenta, ktory jest z nizej
            (board[x][y+1] == DEFENDER || board[x][y+1] == KING) &&
            (board[x][y+2] == ATTACKER || board[x][y+2] == CORNER || board[x][y+2] == THRONE)){
                if(!(board[x][y+1] == KING) || !KING_THRONE){
                    board[x][y+1] = EMPTY;
                }
        }
        if(y > 1 &&  // bicie opponenta, ktory jest z wyzej
            (board[x][y-1] == DEFENDER || board[x][y-1] == KING) &&
            (board[x][y-2] == ATTACKER || board[x][y-2] == CORNER || board[x][y-2] == THRONE)){
                if(!(board[x][y-1] == KING) || !KING_THRONE){
                    board[x][y-1] = EMPTY;
                }
        }
        // bicie krola na tronie
        if(board[4][4] == KING && board[4][3] == ATTACKER && board[4][5] == ATTACKER &&
            board[3][4] == ATTACKER && board[5][4] == ATTACKER){
                board[4][4] = THRONE;
        }
        // bicie krola obok tronu z lewej
        if(board[4][3] == KING && board[4][2] == ATTACKER && board[3][3] == ATTACKER &&
            board[5][3] == ATTACKER){
                board[4][3] = EMPTY;
        }
        // bicie krola obok tronu z prawej
        if(board[4][5] == KING && board[4][6] == ATTACKER && board[3][5] == ATTACKER &&
            board[5][5] == ATTACKER){
                board[4][5] = EMPTY;
        }
        // bicie krola obok tronu u gory
        if(board[3][4] == KING && board[2][4] == ATTACKER && board[3][3] == ATTACKER &&
            board[3][5] == ATTACKER){
                board[3][4] = EMPTY;
        }
        // bicie krola obok tronu z dolu
        if(board[5][4] == KING && board[6][4] == ATTACKER && board[5][3] == ATTACKER &&
            board[5][5] == ATTACKER){
                board[5][4] = EMPTY;
        }
    }
    if(turn == 1){
        // bicie opponenta, ktory jest z prawej
         if(x < BOARD_SIZE - 2 && board[x+1][y] == ATTACKER &&
            (board[x+2][y] == DEFENDER || board[x+2][y] == KING || board[x+2][y] == CORNER || board[x+2][y] == THRONE)){
                board[x+1][y] = EMPTY;
        }
        // bicie opponenta, ktory jest z lewej
        if(x > 1 && board[x-1][y] == ATTACKER &&
            (board[x-2][y] == DEFENDER || board[x-2][y] == KING || board[x-2][y] == CORNER || board[x-2][y] == THRONE)){
                board[x-1][y] = EMPTY;
        }
        // bicie opponenta, ktory jest z nizej
        if(y < BOARD_SIZE - 2 && board[x][y+1] == ATTACKER &&
            (board[x][y+2] == DEFENDER || board[x][y+2] == KING || board[x][y+2] == CORNER || board[x][y+2] == THRONE)){
                board[x][y+1] = EMPTY;
        }
        // bicie opponenta, ktory jest z wyzej
        if(y > 1 && board[x][y-1] == ATTACKER &&
            (board[x][y-2] == DEFENDER || board[x][y-2] == KING || board[x][y-2] == CORNER || board[x][y-2] == THRONE)){
                board[x][y-1] = EMPTY;
        }
    }

}

// 0 - atakujacy, 1 - obronca
int check_victory(char board[BOARD_SIZE][BOARD_SIZE]) {
    if (board[0][0] == KING || board[0][8] == KING || board[8][0] == KING || board[8][8] == KING) {
        return 1;
    }

    int king_x = -1, king_y = -1;
    for(int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == KING) {
                king_x = i;
                king_y = j;
                break;
            }
        }
    }
    if (king_x == -1 || king_y == -1) {
        return 0;
    }

    return 2;
}

void *threadGame(void *arg) {
    struct client_pair *pair = (struct client_pair *)arg;
    int client1 = pair->client1;
    int client2 = pair->client2;
    char buff[100];
    int n;

    char board[BOARD_SIZE][BOARD_SIZE];

    memset(board, EMPTY, sizeof(board));

    board[4][4] = THRONE;
    board[0][0] = CORNER;
    board[0][8] = CORNER;
    board[8][0] = CORNER;
    board[8][8] = CORNER;

    board[4][4] = KING;
    board[4][2] = DEFENDER;
    board[4][3] = DEFENDER;
    board[4][5] = DEFENDER;
    board[4][6] = DEFENDER;
    board[2][4] = DEFENDER;
    board[3][4] = DEFENDER;
    board[5][4] = DEFENDER;
    board[6][4] = DEFENDER;
  
    board[3][0] = ATTACKER;
    board[4][0] = ATTACKER;
    board[5][0] = ATTACKER;
    board[4][1] = ATTACKER;
    board[3][8] = ATTACKER;
    board[4][8] = ATTACKER;
    board[5][8] = ATTACKER;
    board[4][7] = ATTACKER;
    board[0][3] = ATTACKER;
    board[0][4] = ATTACKER;
    board[0][5] = ATTACKER;
    board[1][4] = ATTACKER;
    board[8][3] = ATTACKER;
    board[8][4] = ATTACKER;
    board[8][5] = ATTACKER;
    board[7][4] = ATTACKER;


    //Losowanie, który z graczy jest atakującym.
    srand(time(NULL));
    int attack_player = (rand() % 2) + 1; 
    printf("Atakujący gracz: %d\n", attack_player);
    if(attack_player == 1){
        bzero(buff, sizeof(buff));
        strcpy(buff, "side a");
        if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
            printf("Utracono połączenie z graczem 1\n");
            close(client1);
            close(client2);
            free(pair);
            pthread_exit(NULL);
        }
        bzero(buff, sizeof(buff));
        strcpy(buff, "side d");
        if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
            printf("Utracono połączenie z graczem 2\n");
            close(client1);
            close(client2);
            free(pair);
            pthread_exit(NULL);
        }
    }else{
        bzero(buff, sizeof(buff));
        strcpy(buff, "side a");
        if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
            printf("Utracono połączenie z graczem 1\n");
            close(client1);
            close(client2);
            free(pair);
            pthread_exit(NULL);
        }
        bzero(buff, sizeof(buff));
        strcpy(buff, "side d");
        if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
            printf("Utracono połączenie z graczem 2\n");
            close(client1);
            close(client2);
            free(pair);
            pthread_exit(NULL);
        }
    }
    // potwiedzenie odbioru wiad od gracza 1
    if (!confirm_receipt(client1, "Gracz 1")) {
        close(client1);
        close(client2);
        free(pair);
        pthread_exit(NULL);
    }
    // potwiedzenie odbioru wiad od gracza 2
    if (!confirm_receipt(client2, "Gracz 2")) {
        close(client1);
        close(client2);
        free(pair);
        pthread_exit(NULL);
    }

    int turn = 0; // 0 - atakujący, 1 - obrońca
    bool wrong_move = false;

    while (true) {
        //wysłanie planszy
        if(!send_board(board, client1, client2)){
            break;
        };
        // potwiedzenie odbioru wiad od gracza 1
        if (!confirm_receipt(client1, "Gracz 1")) {
            break;
        }
        // potwiedzenie odbioru wiad od gracza 2
        if (!confirm_receipt(client2, "Gracz 2")) {
            break;
        }

        if(wrong_move){
            bzero(buff, sizeof(buff));
            strcpy(buff, "msg Nieprawidlowy ruch. Sprobuj ponownie.");
            if(turn == 0 && attack_player == 1){
                if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
                // potwiedzenie odbioru wiad od gracza 1
                if (!confirm_receipt(client1, "Gracz 1")) {
                    break;
                }
            }else if(turn == 0 && attack_player == 2){
                if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
                // potwiedzenie odbioru wiad od gracza 2
                if (!confirm_receipt(client2, "Gracz 2")) {
                    break;
                }
            }else if(turn == 1 && attack_player == 1){
                if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
                // potwiedzenie odbioru wiad od gracza 2
                if (!confirm_receipt(client2, "Gracz 2")) {
                    break;
                }
            }else if(turn == 1 && attack_player == 2){
                if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
                // potwiedzenie odbioru wiad od gracza 1
                if (!confirm_receipt(client1, "Gracz 1")) {
                    break;
                }
            }
            wrong_move = false;
        }


        if(turn == 0){//Runda atakującego 
            if(attack_player == 1){ // Atakującym jest 1 gracz
                bzero(buff, sizeof(buff));
                strcpy(buff, "move a");
                if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
                bzero(buff, sizeof(buff));
                strcpy(buff, "msg Ruch przeciwnika");
                if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 2\n");
                    break;
                }
            }else{                  // Atakującym jest 2 gracz
                bzero(buff, sizeof(buff));
                strcpy(buff, "move a");
                if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 2\n");
                    break;
                }
                bzero(buff, sizeof(buff));
                strcpy(buff, "msg Ruch przeciwnika");
                if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
            }
        }else if (turn == 1)//Runda obrońcy 
        {
            if(attack_player == 1){ // Obrońcą jest 2 gracz
                bzero(buff, sizeof(buff));
                strcpy(buff, "move d");
                if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
                bzero(buff, sizeof(buff));
                strcpy(buff, "msg Ruch przeciwnika");
                if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 2\n");
                    break;
                }
            }else{                  // Obrońcą jest 1 gracz
                bzero(buff, sizeof(buff));
                strcpy(buff, "move d");
                if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 2\n");
                    break;
                }
                bzero(buff, sizeof(buff));
                strcpy(buff, "msg Ruch przeciwnika");
                if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                    printf("Utracono połączenie z graczem 1\n");
                    break;
                }
            }
        }
         // potwiedzenie odbioru wiad od gracza 1
        if (!confirm_receipt(client1, "Gracz 1")) {
            break;
        }
        // potwiedzenie odbioru wiad od gracza 2
        if (!confirm_receipt(client2, "Gracz 2")) {
            break;
        }

        //Odbieranie wiadomości zawierającej ruch od gracza
        char ch_y, ch_new_y;
        int x, new_x;
        bzero(buff, sizeof(buff));

        if(turn == 0 && attack_player == 1){
            n = read(client1, buff, sizeof(buff));
            if (n <= 0) {
                printf("Gracz 1 rozlaczony\n");
                break;
            }
        }else if(turn == 0 && attack_player == 2){
            n = read(client2, buff, sizeof(buff));
            if (n <= 0) {
                printf("Gracz 2 rozlaczony\n");
                break;
            }
        }else if(turn == 1 && attack_player == 1){
            n = read(client2, buff, sizeof(buff));
            if (n <= 0) {
                printf("Gracz 2 rozlaczony\n");
                break;
            }
        }else if(turn == 1 && attack_player == 2){
            n = read(client1, buff, sizeof(buff));
            if (n <= 0) {
                printf("Gracz 1 rozlaczony\n");
                break;
            }
        }
        sscanf(buff, "%d %c %d %c", &x, &ch_y, &new_x, &ch_new_y);
        
        x = x - 1; 
        new_x = new_x - 1;
        // Konwersja liter kolumn na indeksy
        int y = ch_y - 'A'; 
        int new_y = ch_new_y - 'A'; 

        //sprawdzanie poprawnosci ruchu i wykonywanie
        if (move_validation(board, x, y, new_x, new_y, turn)) {
            make_move(board, x, y, new_x, new_y);
            battle(board, new_x, new_y, turn);
            int result = check_victory(board);
            if (result == 0) { //Wygrywa atakujący
                //wysłanie planszy
                if(!send_board(board, client1, client2)){
                    break;
                };
                // potwiedzenie odbioru wiad od gracza 1
                if (!confirm_receipt(client1, "Gracz 1")) {
                    break;
                }
                // potwiedzenie odbioru wiad od gracza 2
                if (!confirm_receipt(client2, "Gracz 2")) {
                    break;
                }

                // Wysłanie do graczy informacji o końcu gry
                if(attack_player == 1){ // Atakującym jest 1 gracz
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Wygrałeś!");
                    if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 1\n");
                        break;
                    }
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Przeciwnik wygrał");
                    if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 2\n");
                        break;
                    }
                }else{                  // Atakującym jest 2 gracz
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Wygrałeś!");
                    if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 2\n");
                        break;
                    }
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Przeciwnik wygrał");
                    if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 1\n");
                        break;
                    }
                }
                break; // Koniec gry i wątku.
            }else if(result == 1){
                //wysłanie planszy
                if(!send_board(board, client1, client2)){
                    break;
                };
                // potwiedzenie odbioru wiad od gracza 1
                if (!confirm_receipt(client1, "Gracz 1")) {
                    break;
                }
                // potwiedzenie odbioru wiad od gracza 2
                if (!confirm_receipt(client2, "Gracz 2")) {
                    break;
                }

                // Wysłanie do graczy informacji o końcu gry
                if(attack_player == 1){ // Obrońcą jest 2 gracz
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Wygrałeś!");
                    if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 1\n");
                        break;
                    }
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Przeciwnik wygrał");
                    if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 2\n");
                        break;
                    }
                }else{                  // Obrońcą jest 1 gracz
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Wygrałeś!");
                    if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 2\n");
                        break;
                    }
                    bzero(buff, sizeof(buff));
                    strcpy(buff, "msg Przeciwnik wygrał");
                    if (send(client2, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
                        printf("Utracono połączenie z graczem 1\n");
                        break;
                    }
                }
                break; // Koniec gry i wątku.
            }
            if(turn == 0){
                turn = 1;
            }else{
                turn = 0;
            }
        } else {
            wrong_move = true;
        }
    }

    close(client1);
    close(client2);
    free(pair);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    struct sockaddr_in sa;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1) {
        perror("Nie udalo sie stworzyc socketa");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    
    int port = 1111;
    if (argc > 1){
        port = atoi(argv[1]);
    }
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htons(INADDR_ANY);
    sa.sin_port = htons(port);

    const int one = 1;
    setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (bind(SocketFD, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("Nie udalo sie zrobic bind");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (listen(SocketFD, 10) == -1) {
        perror("Niepowodzenie nasluchiwania");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    pthread_t thread_id;
    char buff[100];
    while (1) {
        struct sockaddr_in ca1, ca2;
        socklen_t sl1 = sizeof(ca1), sl2 = sizeof(ca2);
        // Dołączanie 1 gracza
        int client1 = accept(SocketFD, (struct sockaddr *)&ca1, &sl1);
        if (client1 == -1) {
            perror("Nie udany accept gracza 1");
            continue;
        }
        printf("Połączono z graczem 1: %s, port %d\n", inet_ntoa(ca1.sin_addr), ntohs(ca1.sin_port));

        bzero(buff, sizeof(buff));
        strcpy(buff, "msg Oczekiwanie na dołączenie drugiego gracza...");
        if (send(client1, buff, strlen(buff), MSG_NOSIGNAL) == -1) {
            printf("Utracono połączenie z graczem 1\n");
            close(client1);
            continue;
        }

        // potwiedzenie odbioru wiad od gracza 1
        if (!confirm_receipt(client1, "Gracz 1")) {
            close(client1);
            continue;
        }

        // Dołączanie 2 gracza
        int client2 = accept(SocketFD, (struct sockaddr *)&ca2, &sl2);
        if (client2 == -1) {
            perror("Nie udany accept gracza 2");
            close(client1);
            continue;
        }
        printf("Polaczono z graczem 2: %s, port %d\n", inet_ntoa(ca2.sin_addr), ntohs(ca2.sin_port));

        struct client_pair *pair = malloc(sizeof(struct client_pair));
        pair->client1 = client1;
        pair->client2 = client2;

        if (pthread_create(&thread_id, NULL, threadGame, pair) == -1) {
            perror("Nie udalo sie stworzyc watku");
            close(client1);
            close(client2);
            free(pair);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(SocketFD);
    printf("KONIEC");
    return EXIT_SUCCESS;
}
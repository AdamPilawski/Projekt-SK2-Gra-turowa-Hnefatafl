#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

#define BOARD_SIZE 9
#define THRONE 'T'
#define CORNER 'C'
#define EMPTY '.'
#define KING 'K'
#define DEFENDER 'D'
#define ATTACKER 'A'

char board[BOARD_SIZE][BOARD_SIZE];

void print_board() {
    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%c ", i+65);
    }
    printf("\n");

    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d ", i+1);
        for (int j = 0; j < BOARD_SIZE; j++) {
            char position = board[i][j];
            switch (position) {
                case EMPTY:
                    printf(". ");
                    break;
                case CORNER:
                    printf("X ");
                    break;
                case THRONE:
                    printf("X ");
                    break;
                case KING:
                    printf("\033[1;33mK \033[0m"); // Żółty dla króla
                    break;
                case DEFENDER:
                    printf("\033[1;34mO \033[0m"); // Niebieski dla obrońcy
                    break;
                case ATTACKER:
                    printf("\033[1;31mA \033[0m"); // Czerwony dla atakującego
                    break;
            }
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]){
    struct sockaddr_in sa;
    int SocketFD= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(SocketFD == -1){
        perror("Nie udalo sie stworzyc socketa");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));

    int port = 1111;
    if(argc <= 1){
        printf("Nie podano adresu ip!\n");
        exit(EXIT_FAILURE);
    }
    if (argc > 2){
        port = atoi(argv[2]);
    }

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(argv[1]);
    sa.sin_port = htons(port);

    if(connect(SocketFD, (struct sockaddr*)&sa, sizeof(sa)) == -1){
        perror("Nie udalo sie polaczyc");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    char buff[100];
    char temp[100];
    char ok[5] = "ok";
    int n;
    char side[5];
    while(1){
        bzero(buff, sizeof(buff));
        n = read(SocketFD, buff, sizeof(buff));
        if(n == 0){
            printf("Utracono połączenie z serwerem gry.\n");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        char *space = strchr(buff, ' '); // Znalezienie pierwszej spacji, *space wskaźnik na nią

        if (space != NULL) {
            *space = '\0'; //oddzielenie części informacyjnej od wiadmości
            if (strcmp(buff, "side") == 0) { //odebranie informacji o stronie w grze
                strcpy(side, space + 1);
                printf("%s\n", side);
                if(send(SocketFD, ok, sizeof(ok), MSG_NOSIGNAL) == -1){
                    printf("Utracono połączenie z serwerem gry.\n");
                    close(SocketFD);
                    exit(EXIT_FAILURE);
                }
            }else if (strcmp(buff, "msg") == 0) { //odebranie wiadmosci i jej wypisanie
                bzero(temp, sizeof(temp));
                strcpy(temp, space + 1);
                printf("%s\n", temp);
                if(send(SocketFD, ok, sizeof(ok), MSG_NOSIGNAL) == -1){
                    printf("Utracono połączenie z serwerem gry.\n");
                    close(SocketFD);
                    exit(EXIT_FAILURE);
                }
            }else if (strcmp(buff, "board") == 0) { //odebranie planszy i jej wyświetlenie
                bzero(temp, sizeof(temp));
                strcpy(temp, space + 1);
                for (int i = 0; i < BOARD_SIZE; i++) {
                    for (int j = 0; j < BOARD_SIZE; j++) {
                        board[i][j] = temp[i * BOARD_SIZE + j]; 
                    }
                }
                system("clear"); //wyczyszczenie konsoli
                printf("Grasz jako ");
                if((strcmp(side, "a") == 0)){
                    printf("\033[1;31mAtakujący\n\033[0m"); // Czerwony dla atakującego
                }else if((strcmp(side, "d") == 0)){
                    printf("\033[1;34mObrońca\n\033[0m"); // Niebieski dla obrońcy
                }
                print_board();
                if(send(SocketFD, ok, sizeof(ok), MSG_NOSIGNAL) == -1){
                    printf("Utracono połączenie z serwerem gry.\n");
                    close(SocketFD);
                    exit(EXIT_FAILURE);
                }
            }else if (strcmp(buff, "move") == 0) {
                if(send(SocketFD, ok, sizeof(ok), MSG_NOSIGNAL) == -1){
                    printf("Utracono połączenie z serwerem gry.\n");
                    close(SocketFD);
                    exit(EXIT_FAILURE);
                }
                char ch_y, ch_new_y;
                int x, new_x;
                printf("Twój ruch\n");
                printf("format: kolumna wiersz nowa_koumna nowy_wiersz\n");
                while(true) {
                    char input[10];
                    if (fgets(input, sizeof(input), stdin) == NULL) {
                        printf("Niepoprawny format! Spróbuj ponownie.\n");
                        continue;
                    }
                    if (sscanf(input, "%c %d %c %d", &ch_y, &x, &ch_new_y, &new_x) == 4) {
                        ch_y = toupper((unsigned char) ch_y);
                        ch_new_y = toupper((unsigned char) ch_new_y);
                        if ((ch_y >= 'A' && ch_y <= 'I') && (ch_new_y >= 'A' && ch_new_y <= 'I') &&
                            (x >= 1 && x <= 9) && (new_x >= 1 && new_x <= 9)) {
                            break; 
                        } else {
                            printf("Niepoprawny format! Spróbuj ponownie.\n");
                        }
                    }else {
                        printf("Niepoprawny format! Spróbuj ponownie.\n");
                    }
                }
                bzero(buff, sizeof(buff));
                sprintf(buff, "%d %c %d %c", x, ch_y, new_x, ch_new_y);
                if(send(SocketFD, buff, sizeof(buff), MSG_NOSIGNAL) == -1){
                    printf("Utracono połączenie z serwerem gry.\n");
                    close(SocketFD);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    close(SocketFD);
    return EXIT_SUCCESS;
}
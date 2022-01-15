#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

int main(int argc, char *argv[])
{
    // on test si notre utilisateur a passe assez d'argument ou pas 
    if (argc != 2) {
        fprintf(stderr, "Not enough arguments: Should be './lock filename'\n");
        exit(EXIT_FAILURE);
    } 

    // Tout d'abord, on definit les variables necessaires
    int cmd, start, length;
    struct flock fl; // structure de flock deja definie
    char input[30];
    char cmd_input, ltype_input;
    char whence_input = 's';
    int start_input, lenght_input;
        
    // On test si notre fichier peut etre ouvert, pour eviter d'agir sur un fichier non existant, qui nous creera juste des problemes
    int fd = open(argv[1], O_RDWR); // cette variable va nous permettre de ouvrir le fichier voulu
    if (fd < 0) {
        fprintf(stderr, "Can't open the file '%s': %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE); // dans le cas ou l'on peut pas ouvrir notre fichier, on quitte notre programme en ayant un echec
    }


    for (;;) {  /* Prompt for locking command and carry it out */
        
        printf("Enter ? for help\n");
        printf("Enter exit to close the program\n");
        printf("\n");
        printf("PID=%ld> ",(long) getpid());
        fflush(stdout);

        // use fgets to read user input and then handle it
        fgets(input, sizeof(input), stdin);

        // on va creer un else if qui vas determiner que doit faire le programme en fonction de l'input ecrit par l'utilisateur
    
        if(strcmp(input, "?\n") == 0){ // on utilise la fonction 'strcmp' qui va nous permettre de comparer l'input et ce que l'on veux (ex: ici on veux afficher le 'help' de ce programme)
            printf("\n");
            printf("    Format: cmd l_type start length [whence(optional)]\n");
            printf("    'cmd' ---'g' (F_GETLK), 's' (F_SETLK), or 'w' (F_SETLKW)\n");
            printf("    'l_type' ---'r' (F_RDLCK), 'w' (F_WRLCK), or 'u' (F_UNLCK)\n");
            printf("    'start' ---lock startingoffset\n");
            printf("    'length' ---number of bytes to lock\n");
            printf("    'whence' ---'s' (SEEK_SET, default), 'c' (SEEK_CUR), or 'e' (SEEK_END)\n");
            printf("\n");
 
        }else if(strcmp(input, "exit\n") == 0){ // meme chose que pour le if au dessus, sauf que cette fois on va quitter le programme si jamais l'utilisateur tape 'exit'
            close(fd);
            exit(0);
        }else{ // dans le cas ou aucun des deux strcmp est sollicite, on va pouvoir utiliser notre programme
            sscanf(input, "%c %c %d %d %c", &cmd_input, &ltype_input, &start_input, &lenght_input, &whence_input); // on va utiliser la fonction 'sscanf' pour separer l'input de l'utlisateur 
            printf("\n");
            printf("cmd: %c\n", cmd_input); // on attribue une variable pour chaque input, qu'il soit facultatif ou non
            printf("l_type: %c\n", ltype_input);
            printf("start: %d\n", start_input);
            printf("lenght: %d\n", lenght_input);
            printf("whence: %c\n", whence_input);
            printf("\n");
        }

        
        // process user unput into the 'cmd' variable and the various elements of 'fl' struct
        /* On va utiliser un switch case pour, en fonction de ce que nos variabes input ont stocke, les convertir en flag */
        
        // On convertit le cmd en flag
        switch(cmd_input){ // 
            case 'g' :
                cmd = F_GETLK;
                break;
            case 's' :
                cmd = F_SETLK;
                break;
            case 'w' :
                cmd = F_SETLKW;
                break;
        }

        // on convertit le ltype en flag
        switch(ltype_input){ 
            case 'r' :
                fl.l_type = F_RDLCK;
                break;
            case 'w' :
                fl.l_type = F_WRLCK;
                break;
            case 'u' :
                fl.l_type = F_UNLCK;
                break;
        }

        fl.l_start = start_input; // ici, commme pour lenght, on a pas besoin de switch case, mais on doit juste attribuer l'input a la structure flock
        fl.l_len = lenght_input;

        // On convertit le whence en flag
        switch(whence_input) { 
            case 's' : 
                fl.l_whence = SEEK_SET;
                break;
            case 'c' :
                fl.l_whence = SEEK_CUR;
                break;
            case 'e' :
                fl.l_whence = SEEK_END;
                break;
            default:
                fl.l_whence = SEEK_SET;
                break;
        }
        
        
        int status = fcntl(fd, cmd, &fl);       
        
        // Interpret result of request and inform user
        if (cmd == F_GETLK) { // F_GETLK
            // check status and handle errors
            if (status == 0) {
                // process results and print informative text
                if (fl.l_type == F_UNLCK) { // si on a pas de verrous existant, on va pouvoir en debloquer un
                    printf("[PID=%ld] can place a lock\n", (long)  getpid());
                }else{
                    if(fl.l_type == F_RDLCK) { // cas ou l'on a deja un verrou partage existant
                        printf("[PID=%ld] Cannot place shared lock on %lu:%lu (held by PID %d)\n", (long)  getpid(), fl.l_start, fl.l_len, fl.l_pid);
                    }else if(fl.l_type == F_WRLCK){ // cas ou l'on a un verrou exclusif existant
                        printf("[PID=%ld] Cannot place exclusive lock on %lu:%lu (held by PID %d)\n", (long)  getpid(), fl.l_start, fl.l_len, fl.l_pid);
                    }
                }
            }

        } else if(errno == EINVAL) {
            printf("Invalid argument\n");
        }else { // F_SETLK, F_SETLKW
        // check status and handle  errors  (look  at manual  for  possible errors)
            if  (status == 0 ) {
            // process results and  print informative  text
                if(cmd == F_SETLK){
                    if (fl.l_type == F_UNLCK) {
                    printf("[PID=%ld] unlocked\n", (long)  getpid()); // on debloque le verrou qui ete deja place
                    }else{
                        if(fl.l_type == F_RDLCK) { // on place un verrou partage 
                            printf("[PID=%ld] Set read (shared) lock\n", (long)  getpid());
                        }else if(fl.l_type == F_WRLCK){ // on place un verrou exlusif
                            printf("[PID=%ld] Set write (exclusive) lock\n", (long)  getpid());
                        }

                    }
                } else if(cmd == F_SETLKW) {
                    if(fl.l_type == F_RDLCK){
                        printf("[PID=%ld] Set wait read lock  \n", (long) getpid());
                    } 
                    if(fl.l_type == F_WRLCK){
                        printf("[PID=%ld] Set wait write lock  \n", (long) getpid());
                    }
                    if(fl.l_type == F_UNLCK) {
                        printf("[PID=%ld] Unlock wait lock \n", (long) getpid());
                    }
                }
                
            }else if ((errno == EACCES || errno == EAGAIN)) {
                fprintf(stderr, "Couldn't access lock: %s\n", strerror(errno)); // cas ou l'on a deja un autre processus qui a place un verrou 
            }
        }
    }
    return 0;
}

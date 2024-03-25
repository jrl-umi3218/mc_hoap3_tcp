#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char * argv[]) {

struct mesg{
        double donnee[21];
};

    struct mesg_init{
        double donnee[2];
    };


    struct mesg_RT{
        short donnee[21];
    };

struct mesg message;
struct mesg_init message_init;
    struct mesg_RT message_RT;
    int donnee_q[21];
char messg[10];
double m1, test;
int i, j, err, client,client_init, serveur,serveur_init, nsend, result, nconnect,portcom;
struct sockaddr_in sockAddr, sock;
int longaddr;
   int CHECK_TIME=4;
    int CYCLE_TIME=2;
    int FINBOUCLE;

    serveur_init=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    sockAddr.sin_family=PF_INET;
    sockAddr.sin_port=htons(2600);
    sockAddr.sin_addr.s_addr=0;
    longaddr=sizeof(sockAddr);
    printf("\n server init running ...\n");
    err=bind(serveur_init,(struct sockaddr*)&sockAddr,longaddr);
    if(err==-1) printf(" bind error !! \n");
    err=listen(serveur_init,1);
    if(err==-1) printf("listen error !! \n");
    client_init=accept(serveur_init,(struct sockaddr*)&sock,&longaddr);
    result=read(client_init,&message_init,sizeof(message_init));
    close(serveur_init);

    CYCLE_TIME=(int)message_init.donnee[0];
    CHECK_TIME=(int)message_init.donnee[1];

     printf("Données Initialisation ok, CYCLE_TIME=%d, CHECK_TIME=%d \n", CYCLE_TIME,CHECK_TIME);

    FINBOUCLE=(CHECK_TIME*1000/CYCLE_TIME);

    printf("FINBOUCLE=%d \n", FINBOUCLE);

    portcom=atoi(argv[1]);
    //printf(" Port d'écoute %s\n",argv[1]);
    printf(" Port d'écoute %d\n",portcom);

	serveur=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	sockAddr.sin_family=PF_INET;
 	sockAddr.sin_port=htons(portcom);
 	sockAddr.sin_addr.s_addr=0;
 	longaddr=sizeof(sockAddr);
	printf("\n server running ...\n");


 	err=bind(serveur,(struct sockaddr*)&sockAddr,longaddr);
 	if(err==-1) printf(" bind error !! \n");
 	err=listen(serveur,1);
 	if(err==-1) printf("listen error !! \n");

	/* ouverture du serveur */
    //FINBOUCLE=CHECK_TIME*1000/CYCLE_TIME +1;

 	client=accept(serveur,(struct sockaddr*)&sock,&longaddr);

    //fcntl(serveur,F_SETFL,fcntl(serveur,F_GETFL) | O_NONBLOCK);

i=0;

 do {

    result=read(client,&message,sizeof(message));

    for (j=0;j<21;j++) message_RT.donnee[j]=(short)message.donnee[j];
    for (j=0;j<21;j++) message.donnee[j]=(double)message_RT.donnee[j];
     printf("i=%d \n",i);
     for (j=0;j<21;j++) printf("%1.1lf | ",message.donnee[j]);
     printf("\n ");

         result=write(client,&message,sizeof(message));

         usleep(1000);
         i++;
} while (i<FINBOUCLE);
        // nsend=write(serveur,&message,sizeof(message));

        close(serveur);

}

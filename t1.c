/* Redes de Computadores - Engenharia de Computação
   1º Semestre de 2015 - Prof Julio Estrella
   Aplicação P2P para troca de mensagens

   Lucas Tognoli Munhoz - 8504330
   Rafael Martins de Freitas - 7991893
*/

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <locale.h>

/** Base de dados de Contatos **/

// O pronteiro para a lista de contatos é global
struct Mensagems{
   char *date;
   char *msg_text;
   struct Mensagems *next;
};
typedef struct Mensagems Mensagems;

struct Contacts{
	char *name;
	char *ip;
	struct Contacts *next;		//Implementação utiliza lista encadeada.
   	struct Mensagems *msgs;
};
typedef struct Contacts Contacts;

Contacts *contacts;


void add_contact( char *name, char *ip, Contacts *p){
   
   Contacts *new_contact = (Contacts*)malloc(sizeof(Contacts));		//Aloca um nó da lista
   new_contact->name = (char*)malloc(sizeof(name));		//Aloca o tamanho do nome.
   strcpy(new_contact->name,name);
   new_contact->ip = (char*)malloc(sizeof(ip));		//Aloca o tamanho do ip.
   strcpy(new_contact->ip,ip);
   
   Mensagems *msgs = (Mensagems*)malloc(sizeof(Mensagems));
   new_contact->msgs = msgs;

   new_contact->next = p->next;		//Insere no começo da fila.
   p->next = new_contact;
}

void add_msg(char *date, char *msg_text, Mensagems *p){
  
   Mensagems *new_msgs = (Mensagems*)malloc(sizeof(Mensagems));
   new_msgs->date = (char*)malloc(sizeof(date));
   strcpy(new_msgs->date,date);
   new_msgs->msg_text = (char*)malloc(sizeof(msg_text));  
   strcpy(new_msgs->msg_text,msg_text);
   
   new_msgs->next = p->next;    
   p->next = new_msgs;
}
//Realiza busca nos contatos pelo nome.
Contacts* search_name( char *name, Contacts *ini){
   Contacts *p;
   p = ini->next;
   while (p != NULL && strcmp(p->name,name) != 0)
      p = p->next;
   return p;
}
//Realiza busca nos contatos pelo IP.
Contacts* search_ip( char *ip, Contacts *ini){
   Contacts *p;
   p = ini->next;
   while (p != NULL && strcmp(p->ip,ip) != 0)
      p = p->next;
   return p;
}
//Imprime todos contatos.
void print(Contacts *p){
	p = p->next;
	while (p != NULL){
		printf ("Name: %s  IP: %s \n",p->name,p->ip);
		p = p->next;
	}
}
void print_msg(Mensagems *p){
	p = p->next;
	while (p != NULL){
		printf ("Date: %s\nMsg: %s \n",p->date,p->msg_text);
		p = p->next;
	}
}
//Remove o contato da lista pelo nome.
void remove_name(char *name,Contacts *ini){
   Contacts *p, *q;
   p = ini;
   q = ini->next;
   while (q != NULL && strcmp(q->name,name) != 0) {
      p = q;
      q = q->next;
   }
   if (q != NULL) {
      p->next = q->next;
      free( q);
   }
}
//Remove contato pelo IP
void remove_ip(char *ip,Contacts *ini){
   Contacts *p, *q;
   p = ini;
   q = ini->next;
   while (q != NULL && strcmp(q->ip,ip) != 0) {
      p = q;
      q = q->next;
   }
   if (q != NULL) {
      p->next = q->next;
      free( q);
   }
}

/**  CLIENTE   **/

// Inicia uma conexão (socket) e envia a mensagem para o servidor
void cli_send(char* server_name){

   int sock, bytes_recv, i;
   char send_data[1024],recv_data[1024];
   struct hostent *host;
   struct sockaddr_in server_addr;

   host = gethostbyname(server_name);

   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      printf("\nErro de Socket");
      printf("\n\n\n");
      sleep(2);
      return;
   }

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(7000);
   server_addr.sin_addr = *((struct in_addr *)host->h_addr);
   bzero(&(server_addr.sin_zero),8);

   if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
      printf("\nErro de conexao! O host esta offline");
      printf("\n\n\n");
      sleep(3);
      return;
   }

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Enviar Mensagem\n\n");
    printf("Mensagem para %s:\n\n", server_name);

    while(1){
        printf("\nYou: ");
        fflush(stdin);
        fgets(send_data, 1024, stdin);
        send(sock,send_data,strlen(send_data), 0);
        sleep(2);
        if(strcmp(send_data, "quit\n") == 0 || strcmp(send_data, "Quit\n") == 0 || strcmp(send_data, "QUIT\n") == 0 )
            break;
            
    }
    printf("Mensagens enviadas!");
    close(sock);
}


void cli_sendgroup(char* server_name, char send_data[1024]){

   int sock, bytes_recv, i;

   struct hostent *host;
   struct sockaddr_in server_addr;

   host = gethostbyname(server_name);

   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("\nErro de Socket");
      exit(1);
   }

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(7000);
   server_addr.sin_addr = *((struct in_addr *)host->h_addr);
   bzero(&(server_addr.sin_zero),8);

   if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
      perror("\nErro de conexao");
      exit(1);
   }

   send(sock,send_data,strlen(send_data), 0);

   close(sock);
}


/** SERVIDOR **/

void insert_buffer(Contacts *atual, char *send_data){
	add_msg(send_data,send_data,atual->msgs);
}




void serv_main(){

   int sock, connected, bytes_recv, i, true = 1;
   char send_data [1024] , recv_data[1024];
   struct sockaddr_in server_addr, client_addr;
   int sin_size;

   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
     perror("Erro no Socket");
     exit(1);
   }


   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true,sizeof(int)) == -1)
   {
      perror("Erro Setsockopt");
      exit(1);
   }


   // Configura o endereco de destino
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(7000);
   server_addr.sin_addr.s_addr = INADDR_ANY;
   bzero(&(server_addr.sin_zero),8);


   if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
      perror("Nao foi possivel realizar o bind");
      exit(1);
   }

   if (listen(sock, 10) == -1)
   {
      perror("Erro de Listen");
      exit(1);
   }

   Contacts *atual;
   // Loop para receber varias solicitacoes
   while(1)
   {

      // Variavel para armazenar o tamanho de endereco do cliente conectado
      sin_size = sizeof(struct sockaddr_in);


      connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
      //printf("\nConexão aceita de (%s , %d)\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

      // Loop para manter a troca de mensagens
      while (1){

            // Funcao recv (int socket, void *buffer, size_t size, int flags)
            bytes_recv = recv(connected, recv_data, 1024, 0);
            

            recv_data[bytes_recv] = '\0';
            printf("Mensagem Recebida: %s \n", recv_data);
            sleep(4);

            if (strcmp(recv_data,"quit\n") == 0 || strcmp(recv_data,"Quit\n") == 0 || strcmp(recv_data,"QUIT\n") == 0)
            {	
               close(connected);
               printf("\nThe Cliente enviou uma mensagem de fechamento de conexao!\n");
               fflush(stdout);
               break;
            }
            else{
            	printf("Não caiu no IF\n");
               atual = search_ip(inet_ntoa(client_addr.sin_addr), contacts);
               if(atual == NULL){
                    add_contact("Visitante", inet_ntoa(client_addr.sin_addr), contacts);
                    atual = search_name(inet_ntoa(client_addr.sin_addr), contacts);
                    insert_buffer(atual, recv_data);
               }
               else
                    insert_buffer(atual, recv_data);
           }

           fflush(stdout);
      }
   }

   close(sock);

}

/** MENUS DO CLIENTE **/

void menu_add(){

    char name[512];
    char ip[512];
    char in_keyb[3];

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Adicionar Contato\n\n");
    printf("Nome do Contato: ");

    scanf("%s", name);
    getchar();

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Adicionar Contato\n\n");
    printf("IP do Contato: ");

    scanf("%s", ip);
    getchar();

    system("clear");
    printf("\t\tNepThug Messaging\n\n\n");
    printf("Adicionar Contato\n\n");
    printf("Nome: %s\n", name);
    printf("IP: %s\n", ip);

    printf("\nTem certeza que desejar adicionar esse contato? (s/n) ");
    scanf("%s", in_keyb);
    getchar();

    if(strcmp(in_keyb, "s") == 0){
        add_contact(name, ip, contacts);

        system("clear");
        printf("\t\tNepThug Messaging\n\n\n");
        printf("Adicionar Contato\n");
        printf("\n\nContato adicionado com sucesso!");
        printf("\n");
        sleep(1);
    }
}

void menu_rem(){

    char name[512];
    char ip[512];
    char in_keyb[3];

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Remover Contato\n\n");
    printf("1 - Remover por Nome\n");
    printf("2 - Remover por IP\n");
    printf("3 - Voltar\n");

    scanf("%s", in_keyb);
    getchar();

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Remover Contato\n\n");


        if(strcmp(in_keyb, "1") == 0){
            printf("Digite o nome do contato a ser removido: ");
            scanf("%s", name);
            getchar();
            remove_name(name, contacts);
            printf("\n\nContato removido com sucesso!");
            sleep(2);
        }

        if(strcmp(in_keyb, "2") == 0){
            menu_rem();
            printf("Digite o IP do contato a ser removido: ");
            scanf("%s", ip);
            getchar();
            remove_ip(ip, contacts);
            printf("\n\nContato removido com sucesso!");
            sleep(2);
        }
}

void menu_list(){

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Lista de Contatos\n\n");
    print(contacts);
    getchar();
}

void menu_send(){

    char name[512];

    Contacts *receiver;

    system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Enviar Mensagem\n\n");
    printf("Escolha um contato:\n\n");

    print(contacts);
    printf("\n\nDigite o nome do contato para enviar a mensagem: ");
    scanf("%s", name);
    getchar();

    receiver = search_name(name, contacts);
    cli_send(receiver->ip);
}

void menu_sendgroup(){

	char *name, send_data[1024];
	//char group[50][512];
	char **group;
	int j, i = 0;
	Contacts *aux;

	name = (char*)malloc(512 * sizeof(char));
	group = (char**)malloc(50 * sizeof(char*));

	system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Enviar Mensagem em Grupo\n\n");
    
    printf("Mensagem: ");
    fflush(stdin);
    fgets(send_data, 1024, stdin);
    
    
    printf("\nEscolha os contatos para mandar a mensagem, teclando ENTER apos cada escolha. Digite 'send' para enviar:\n\n");
    
    print(contacts);
    printf("\n");
    
    while(1){
    	
    	printf("%d: ", (i+1));
    	scanf("%s", name);
    	getchar();
    	
    	if (strcmp (name, "send") == 0)
    		break;
    	
    	group[i] = name;
    	i++;
    }
    
    for (j=0; j<i; j++){
    
    	aux = search_name(group[j], contacts);
    	/** IMPLEMENTAR ERRO SE NAO ENCONTRAR O NOME **/
    	cli_sendgroup(aux->name, send_data); 
    }
}

void menu_print_msg(){
   	
	char name[512];
	Contacts *aux;

   	system("clear");

    printf("\t\tNepThug Messaging\n\n\n");
    printf("Enviar Mensagem\n\n");
    printf("Escolha um contato:\n\n");

    print(contacts);
    printf("\n\nDigite o nome do contato para visualizar as mensagens recebidas: ");
    scanf("%s", name);
    getchar();

    aux = search_name(name, contacts);

	system("clear");
	printf("Mensagems de %s:\n\n",aux->name);    
    print_msg(aux->msgs);
    getchar();
}


void cli_main(){

	setlocale(LC_ALL,"");

    char in_keyb[3];

    while(1){

        system("clear");

        printf("\t\tNepThug Messaging\n\n");
        printf("Escolha uma opcao abaixo, digite o numero correspondente e de ENTER:\n\n");
        printf("1 - Adicionar Contato\n");
        printf("2 - Remover Contato\n");
        printf("3 - Listar Contatos\n");
        printf("4 - Enviar uma Mensagem\n");
        printf("5 - Enviar uma Mensagem em Grupo\n");
        printf("6 - Sair\n\n");
        printf("7 - Imprimir Mensagens\n\n");
        printf("Opcao: ");

        scanf("%s", in_keyb);
        getchar();


            if(strcmp(in_keyb, "1") == 0){
                menu_add();
            }

            if(strcmp(in_keyb, "2") == 0){
                menu_rem();
            }

            if(strcmp(in_keyb, "3") == 0){
                menu_list();
            }

            if(strcmp(in_keyb, "4") == 0){
                menu_send();
            }

            if(strcmp(in_keyb, "5") == 0){
                menu_sendgroup();
            }

            if(strcmp(in_keyb, "6") == 0){
                exit(0);
        	}
        	if(strcmp(in_keyb, "7") == 0){
                menu_print_msg();
        	}
    }
}


void main(){

	setlocale(LC_ALL,"");

	contacts = (Contacts*)malloc(sizeof(Contacts));	
	
    pthread_t client, server;

    if(pthread_create(&client, 0, (void*)&cli_main, (void*)0) != 0) {
      printf("Error creating client... exiting!\n");
      exit(EXIT_FAILURE);
    }

    if(pthread_create(&server, 0, (void*)&serv_main, (void*)0) != 0) {
      printf("Error creating client... exiting!\n");
      exit(EXIT_FAILURE);
    }

    pthread_join(client, 0);
    pthread_join(server, 0);

}


/**  menu para broadcast CHECK
 **  buffer de mensagens na struct
 **  pensar no broadcast
 **  fazer a main do cliente CHECK **/

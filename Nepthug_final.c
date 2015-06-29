/* Redes de Computadores - Engenharia de Computacao
   1ro Semestre de 2015 - Prof Julio Estrella
   Aplicacao P2P para troca de mensagens

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
#include <time.h>

/** Base de dados de Contatos **/

struct Mensagens{
	char *date;
	char *msg_text;
	struct Mensagens *next;
};
typedef struct Mensagens Mensagens;
struct Contacts{
	int msgs_to_read;		//Numero de mensagens que ainda nao foram lidas.
	char *name;
	char *ip;
	struct Contacts *next;		//Implementacao utiliza lista encadeada com cabeça.
	struct Mensagens *msgs;
};
typedef struct Contacts Contacts;

Contacts **contacts; //N� raiz da lista de contatos � global.

//verifica se ha 3 'pontos' no endereço e se ha somente numeros
int ip_check (char *ip){
	int i, dots = 0;
	for (i = 0; i < strlen(ip); i++)
	{
		if (ip[i] == '.' || ip[i] == '0' || ip[i] == '1' || ip[i] == '2' || ip[i] == '3' || ip[i] == '4' || ip[i] == '5' || ip[i] == '6' || ip[i] == '7' || ip[i] == '8' || ip[i] == '9'){

		}
		else
			return 0;
		if(ip[i] == '.')
			dots++;	
	}
	if(dots == 3)
		return 1;
	else
		return 0;
}

//adiciona uma mensagem no buffer
void add_msg(char *date, char *msg_text, Mensagens *p){

	Mensagens *new_msgs = (Mensagens*)malloc(sizeof(Mensagens));
	new_msgs->date = (char*)malloc(sizeof(date));
	strcpy(new_msgs->date,date);
	new_msgs->msg_text = (char*)malloc(1024*sizeof(char));
	strcpy(new_msgs->msg_text,msg_text);

	new_msgs->next = p->next;
	p->next = new_msgs;
}
//Realiza busca nos contatos pelo nome.
Contacts* search_name( char *name, Contacts **ini){
	Contacts *p;
	p = *ini;
	while (p != NULL && strcmp(p->name,name) != 0)
		p = p->next;
	return p;
}
//Realiza busca nos contatos pelo IP.
Contacts* search_ip( char *ip, Contacts **ini){
	Contacts *p;
	p = *ini;
	while (p != NULL && strcmp(p->ip,ip) != 0)
		p = p->next;
	return p;
}
//checa se um dado contato esta onlne, abrindo um socket e tentando conectar com ele
char* check_online(Contacts *q, char *local_name){

	char send_data[31], aux_name[25];
	strcpy(send_data, "");
	strcat(send_data,local_name);
	strcat(send_data,"quit\n");

	char str_data[1024];
	int sock, bytes_recv, i;
	struct hostent *host;
	struct sockaddr_in server_addr;

	host = gethostbyname(q->ip);

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
		close(sock);
		return "Offline";
	}
	else{
		send(sock,send_data,strlen(send_data), 0);
		close(sock);
		return "Online ";
	}
}

//Imprime todos contatos.
void print(Contacts **p, char *local_name){
	Contacts *q;
	q = *p;

	printf("+-----------------------------------------------------------------------------------+\n");
	printf("|                                  TABELA DE CONTATOS                               |\n");
	printf("|-----------------------------------------------------------------------------------|\n");
	printf("|            Name            |        IP         |    MSGs NOT READ  |    Status    |\n");
	printf("|----------------------------|-------------------|-------------------|--------------|\n");
	while (1){
		if(q == NULL)
			break;
		printf ("| %25s  |  %16s |       [%3d]       |    %8s  |\n",q->name,q->ip,q->msgs_to_read, check_online(q, local_name));
		q = q->next;
	}
	printf("+-----------------------------------------------------------------------------------+\n");
}

void print_msg(Mensagens *p){
	p = p->next;
	while (p != NULL){
		printf ("[%s]: %s \n",p->date,p->msg_text);
		p = p->next;
	}
}
// adiciona um contato na lista
void add_contact( char *name, char *ip, Contacts **p){

	// nao adiciona se o contato ja estiver na lista
	if(search_name(name, contacts) != NULL || search_ip(ip, contacts) != NULL){
		system("clear");
		printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
		printf("Adicionar Contato\n");
		printf("\n\nContato ja esta na lista!");
		printf("\n");
		sleep(2);   
	}
	else{
	   Contacts *new_contact = (Contacts*)malloc(sizeof(Contacts));		//Aloca um n� da lista
	   new_contact->name = (char*)malloc(sizeof(name));		//Aloca o tamanho do nome.
	   strcpy(new_contact->name,name);
	   new_contact->ip = (char*)malloc(sizeof(ip));		//Aloca o tamanho do ip.
	   strcpy(new_contact->ip,ip);

	   Mensagens *msgs = (Mensagens*)malloc(sizeof(Mensagens));
	   new_contact->msgs = msgs;

	   new_contact->msgs_to_read = 0;

	   new_contact->next = NULL;
	   if(*p == NULL){
	   		*p = new_contact;	
	   }
	   else{
	   		Contacts *q;
	   		q = *p;

	   		while(q->next != NULL){
	   			q = q->next;
	   		}
	   		q->next = new_contact;
	   }
	}
}

//Remove o contato da lista pelo nome.
void remove_name(char *name,Contacts **ini){
	Contacts *p, *q;
	p = *ini;
	q = p->next;
	
	if(strcmp((*ini)->name,name) == 0){
		q = *ini;
		*ini = (*ini)->next;
		free(q);
	}
	else{
		while (q != NULL && strcmp(q->name,name) != 0) {
			p = q;
			q = q->next;
		}
		if (p != NULL) {
			p->next = q->next;
			free(q);
		}
	}
}
//Remove contato pelo IP
void remove_ip(char *ip,Contacts **ini){
	Contacts *p, *q;
	p = *ini;
	q = p->next;
	
	if(strcmp((*ini)->ip,ip) == 0){
		q = *ini;
		*ini = (*ini)->next;
		free(q);
	}
	else{
		while (q != NULL && strcmp(q->ip,ip) != 0) {
			p = q;
			q = q->next;
		}
		if (p != NULL) {
			p->next = q->next;
			free(q);
		}
	}
}

/** SERVIDOR **/
//insere uma mensagem no buffer
void insert_buffer(Contacts *atual, char *send_data){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	char aux[100];
	char date[100] = "";


	sprintf(aux,"%d",tm.tm_mday);
	strcat(date,aux);
	strcat(date,"-");
	sprintf(aux,"%d",tm.tm_mon + 1);
	strcat(date,aux);
	strcat(date,"-");
	sprintf(aux,"%d",tm.tm_year + 1900);
	strcat(date,aux);

	strcat(date," at ");

	sprintf(aux,"%.2d",tm.tm_hour);
	strcat(date,aux);
	strcat(date,":");
	sprintf(aux,"%.2d",tm.tm_min);
	strcat(date,aux);


	atual->msgs_to_read++;

	add_msg(date,send_data,atual->msgs);

}
// função principal da thread listener, responsável por aceitar conexoes, receber mensagens
// e inseri-las no buffer
void serv_main(){

	char name[25],aux_name[25];	//Armazena o nome de quem enviou a mensagem.
	int k=0, aux = 0, j;
	char str_data[1024], data_aux[1024];	//Armazena o conteudo da mensagem
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
		sin_size = sizeof(struct sockaddr_in); // Variavel para armazenar o tamanho de endereco do cliente conectado

		connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
      		// Loop para manter a troca de mensagens
		j = 0;
		while (1){

			strcpy(recv_data,"");
           		 // Funcao recv (int socket, void *buffer, size_t size, int flags)
			bytes_recv = recv(connected, recv_data, 1024, 0);
			i++;

			recv_data[bytes_recv] = '\0';
			strcpy(str_data,"");
            strcpy(aux_name,"");
			strcpy(name,"");
			strcpy(data_aux,"");
			k=0;
			strncpy(data_aux,recv_data+25, 999);
			aux = strlen(data_aux);
			strcat(data_aux,"\0");
			strncpy(str_data,data_aux, aux);

			if (strcmp(data_aux,"quit\n") == 0 || strcmp(data_aux,"Quit\n") == 0 || strcmp(data_aux,"QUIT\n") == 0)
			{
				close(connected);

				fflush(stdout);
				break;
			}
			else{
				atual = search_ip(inet_ntoa(client_addr.sin_addr), contacts);
				// o contato ainda nao esta na lista, deve ser adicionado
				if(atual == NULL){

					strncpy ( aux_name, recv_data, 25 );
					// extrai o nome do emissor da propria mensagem
					while(aux_name[k] != '*'){
						k++;
					}
					strncpy(name,aux_name,k);

					add_contact(name, inet_ntoa(client_addr.sin_addr), contacts);
					atual = search_ip(inet_ntoa(client_addr.sin_addr), contacts);
					insert_buffer(atual, data_aux);
					fflush(stdout);
				}
				else{

					insert_buffer(atual, data_aux);
					fflush(stdout);
				}
			}
			sleep(1);
			fflush(stdout);
		}
	}

	close(sock);
}


/**  CLIENTE   **/

// Inicia uma conex�o (socket) e envia a mensagem para o servidor
void cli_send(char* server_name,char* local_name){

	char str_data[1024];
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

	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	printf("Enviar Mensagem\n\n");
	printf("Mensagem para %s:\n\n", server_name);

	while(1){
		strcpy(send_data,"");
		printf("\nTexto da Mensagem: ");
		fflush(stdin);
		fgets(str_data, 1024, stdin);
		// insere seu nome no 'cabecalho' da mensagem
		strcat(send_data,local_name);
		strcat(send_data,str_data);

		send(sock,send_data,strlen(send_data), 0);

		sleep(2);
		// envia mensagens ate receber o comando 'quit'
		if(strcmp(str_data, "quit\n") == 0 || strcmp(str_data, "Quit\n") == 0 || strcmp(str_data, "QUIT\n") == 0 )
			break;
		fflush(stdin);

	}
	close(sock);
}

// envia uma mensagem para um contato
// a diferença para o send normal eh que aqui so envia uma mensagem, e ja manda o 'quit'
void cli_sendgroup(char* server_name, char* send_data){

	int sock, bytes_recv, i, k=0;
	char name[25], aux_name[25];

	struct hostent *host;
	struct sockaddr_in server_addr;

	host = gethostbyname(server_name);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("\nErro de Socket");
		printf("\n");
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7000);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8);

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("\nErro de conexao! O host esta offline");
		printf("\n");
		return;
	}

	send(sock,send_data,strlen(send_data), 0);
	sleep(2);

	strncpy (aux_name, send_data, 25 );
		while(aux_name[k] != '*'){
			k++;
		}
	strncpy(name,aux_name,k);
	for(k=strlen(name);k<25;k++){
		strcat(name,"*");
	}
	strcpy(send_data,"");
	strcat(send_data, name);
	strcat(send_data,"quit\n");
	send(sock,send_data,strlen(send_data), 0);
	//sleep(3);

	close(sock);
}

/** MENUS DO CLIENTE **/

void menu_add(){

	char name[512];
	char ip[512];
	char in_keyb[3];

	system("clear");

	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	printf("Adicionar Contato\n\n");
	printf("Nome do Contato: ");

	scanf("%s", name);
	getchar();

	system("clear");

	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	printf("Adicionar Contato\n\n");
	printf("IP do Contato: ");

	scanf("%s", ip);
	getchar();

	system("clear");
	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	printf("Adicionar Contato\n\n");
	printf("Nome: %s\n", name);
	printf("IP: %s\n", ip);

	if(ip_check(ip) == 0){
		printf("\nIP invalido!\n");
		sleep(2);
	}
	else{
		printf("\nTem certeza que desejar adicionar esse contato? (s/n) ");
		scanf("%s", in_keyb);
		getchar();

		if(strcmp(in_keyb, "s") == 0){
			add_contact(name, ip, contacts);
			system("clear");
			printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
			printf("Adicionar Contato\n");
			printf("\n\nPronto!");
			printf("\n");
			sleep(1);	
		}
	}
}

void menu_rem(){

	char name[512];
	char ip[512];
	char in_keyb[3];

	system("clear");

	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
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
		if(search_name(name,contacts) == NULL){
			printf("\nContato inexistente.");
			sleep(2);
		}
		else{
			remove_name(name, contacts);
			printf("\n\nContato removido com sucesso!");
			sleep(2);
		}
	}
	if(strcmp(in_keyb, "2") == 0){
		printf("Digite o IP do contato a ser removido: ");
		scanf("%s", ip);
		getchar();
		if(search_ip(ip,contacts) == NULL){
			printf("\nContato inexistente.");
			sleep(2);
		}
		else{
			remove_ip(ip, contacts);
			printf("\n\nContato removido com sucesso!");
			sleep(2);
		}
	}
}

void menu_list(char *local_name){

	system("clear");
	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	print(contacts, local_name);
	getchar();
}

void menu_send(char * local_name){

	char name[512];
	Contacts *receiver;

	while(1){
		system("clear");
		printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
		printf("Enviar Mensagem\n\n");
		printf("Escolha um contato:\n\n");

		print(contacts, local_name);
		printf("\n\nDigite o nome do contato para enviar a mensagem ou 'quit' para sair: ");
		scanf("%s", name);
		getchar();
		receiver = search_name(name, contacts);
		if (receiver == NULL){
			if (strcmp(name,"quit")==0 || strcmp(name,"Quit")==0 || strcmp(name,"QUIT")==0){
				return;
			}
			else{
				printf("\nContato inexistente. ");
				getchar();
			}
		}
		else{
			cli_send(receiver->ip,local_name);
			return;
		}
	}
}

void menu_sendgroup(char* local_name){

	char name[25], send_data[1024];
	char str_data[1014];
	Contacts *aux;
	int i = 0;

	system("clear");

	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	printf("Enviar Mensagem em Grupo\n\n");

	printf("Mensagem: ");
	fflush(stdin);
	fgets(str_data, 1024, stdin);


	printf("\nEscolha os contatos para mandar a mensagem, teclando ENTER apos cada escolha. Digite 'quit' para voltar ao menu:\n\n");

	print(contacts, local_name);
	printf("\n");

	while(1){

		strcpy(send_data,"");
		printf("\n%d: ", (i+1));
		scanf("%s", name);
		getchar();

		if (strcmp(name,"quit")==0 || strcmp(name,"Quit")==0 || strcmp(name,"QUIT")==0)
			break;
		else{
			aux = search_name(name, contacts);
			if(aux == NULL){
				printf("Este contato nao esta adicionado.\n");
			}
			else{
				strcat(send_data,local_name);
				strcat(send_data,str_data);
				cli_sendgroup(aux->ip, send_data);
				//sleep(1);
				i++;
			}
		}
	}
}

void menu_print_msg(char* local_name){
	char name[512];
	Contacts *aux;
	system("clear");

	printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
	printf("Enviar Mensagem\n\n");
	printf("Escolha um contato:\n\n");

	print(contacts, local_name);
	printf("\n\nDigite o nome do contato para visualizar as mensagens recebidas: ");
	scanf("%s", name);
	getchar();

	if (strcmp(name,"quit")==0 || strcmp(name,"Quit")==0 || strcmp(name,"QUIT")==0) return;
	aux = search_name(name, contacts);
	if (aux == NULL) return;
	system("clear");
	printf("Mensagens de %s:\n\n",aux->name);
	aux->msgs_to_read = 0;
	print_msg(aux->msgs);
	getchar();
}


void cli_main(){

	setlocale(LC_ALL,"");

		system("clear");
		int k;
		char local_name[25];
		printf("Insira seu nome:");
		scanf("%s",local_name);
		getchar();
		for(k=strlen(local_name);k<25;k++){
			strcat(local_name,"*");
		}

	char in_keyb[3];

	while(1){

		system("clear");
		printf("\t\t*************  .::NepThug Messaging::. *************\n\n");
		printf("Escolha uma opcao abaixo, digite o numero correspondente e de ENTER:\n\n");
		printf("1 - Adicionar Contato.\n");
		printf("2 - Remover Contato.\n");
		printf("3 - Listar Contatos.\n");
		printf("4 - Enviar uma Mensagem.\n");
		printf("5 - Enviar uma Mensagem em Grupo.\n");
		printf("6 - Ler mensagens.\n");
		printf("7 - Sair.\n\n");
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
			menu_list(local_name);
		}

		if(strcmp(in_keyb, "4") == 0){
			menu_send(local_name);
		}

		if(strcmp(in_keyb, "5") == 0){
			menu_sendgroup(local_name);
		}
		if(strcmp(in_keyb, "6") == 0){
			menu_print_msg(local_name);
		}
		if(strcmp(in_keyb, "7") == 0){
			exit(0);
		}

	}
}

void main(){

	setlocale(LC_ALL,"");

	contacts = (Contacts**)malloc(sizeof(Contacts*));

	*contacts = NULL;

	pthread_t client, server;

	//cria a thread de cliente
	if(pthread_create(&client, 0, (void*)&cli_main, (void*)0) != 0) {
		printf("Error creating client... exiting!\n");
		exit(EXIT_FAILURE);
	}
	// cria a thread listener
	if(pthread_create(&server, 0, (void*)&serv_main, (void*)0) != 0) {
		printf("Error creating client... exiting!\n");
		exit(EXIT_FAILURE);
	}

	pthread_join(client, 0);
	pthread_join(server, 0);
}

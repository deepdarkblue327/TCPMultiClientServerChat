/**
 * @author  Sunil Umasankar <suniluma@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <netdb.h>

using namespace std;

string UBITNAME = "suniluma";
int BACKLOG = 10;
int STDIN = 0;
int BUFFER_SIZE = 512;
int port_value = 0;

#include "../include/global.h"
#include "../include/linkedlist.h"
#include "../include/logger.h"


//Helper Functions

bool ip_validity(string ip) {
	struct sockaddr_in address;
	if (inet_pton(AF_INET, ip.c_str(), &(address.sin_addr)) != 0) {
		return true;
	} else {
		return false;
	}
}

bool is_upper_string(string cmd) {
	int i = 0;
	while(cmd[i]) {
		if(islower(cmd[i])) return false;
		i++;
	}
	return true;
}

string string_input() {
	string cmd;
	cout<<">>";
	cin>>cmd;
	while(!is_upper_string(cmd)) {
		cout<<"Input is not all caps"<<endl;
		cout<<">>";
		cin>>cmd;
	}
	return cmd;
}

void success_log(string cmd) {
	printf("[%s:SUCCESS]\n", cmd.c_str());
}

void error_log(string cmd) {
	printf("[%s:ERROR]\n", cmd.c_str());
}

void end_log(string cmd) {
	printf("[%s:END]\n", cmd.c_str());
}

string get_external_ip() {
	//copied from Stack Overflow
	//https://stackoverflow.com/questions/25879280/getting-my-own-ip-address-by-connecting-using-udp-socket
	int sockfd;
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    char *ip_addr;
    ip_addr = (char*)malloc(sizeof(char) * INET6_ADDRSTRLEN);
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo("8.8.8.8", "http", &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    // loop through all the results and make a socket
    for(p = ai; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
            perror("UDP: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("UDP: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "UDP: failed to bind socket\n");
        exit(2);
    }
	addrlen = sizeof(remoteaddr);
    getsockname(sockfd, (struct sockaddr*)&remoteaddr, &addrlen);

    // deal with both IPv4 and IPv6:
    if (remoteaddr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&remoteaddr;
        inet_ntop(AF_INET, &s->sin_addr, remoteIP, addrlen);
    }
    else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&remoteaddr;
        inet_ntop(AF_INET6, &s->sin6_addr, remoteIP, addrlen);
    }

    freeaddrinfo(ai); // all done with this structure
    close(sockfd);

    strcpy(ip_addr, remoteIP);
	return ip_addr;
}

int common_commands(string command_str) {
	if(command_str == "IP") {
		try {
			string ip_addr = get_external_ip();
			if(ip_addr != "") {
				success_log(command_str);
				printf("IP:%s\n", ip_addr.c_str());
				end_log(command_str);
			}
			else {
				error_log(command_str);
				end_log(command_str);
			}
		} catch (...) {
			error_log(command_str);
			end_log(command_str);
		}
		return 1;
	}
	if(command_str == "PORT") {
		success_log(command_str);
		printf("PORT:%d\n", port_value);
		end_log(command_str);
		return 1;
	}
	return 0;

}

string splitter(string str, string delimiter = " ", int index = 0) {

	size_t pos = 0;
	string token;
	int i = 0;
	while ((pos = str.find(delimiter)) != string::npos) {
	    token = str.substr(0, pos);
	    if (i == index) {
			return token;

		}
	    str.erase(0, pos + delimiter.length());
		i++;
	}
	if(i == index) {
		return str;
	}
	return "";
}

string splitterv2(string str, string delimiter = " ", int index = 0) {

	size_t pos = 0;
	string token;
	int i = 0;
	while ((pos = str.find(delimiter)) != string::npos) {
	    token = str.substr(pos + delimiter.length(), str.length());
	    if (i == index) {
			return token;

		}
	    str.erase(0, pos + delimiter.length());
		i++;
	}
	if(i == index) {
		return str;
	}
	return "";
}


//Server Side Code
string get_client_ip(int server, struct sockaddr_in client) {
	int addrlen;
	addrlen = sizeof(client);
	getpeername(server, (struct sockaddr*)&client,(socklen_t*)&addrlen);
	return inet_ntoa(client.sin_addr);
}

char * get_domain_from_ip(sockaddr_in client) {
	struct hostent *hp;
	hp = gethostbyaddr((const void *)&client.sin_addr, sizeof(client.sin_addr), AF_INET);
	return hp->h_name;
}

int server_program(int port) {
	port_value = port;

	struct Node * head = NULL;

	socklen_t addrlen;
	char buff[BUF];
	int server, head_socket, select_return, accept_fd = 0, sock_index;
	fd_set master_list, watch_list;
	struct sockaddr_in server_address, client_address;

	server = socket(AF_INET, SOCK_STREAM, 0);
	if(server < 0) {
		cout<<"Error connecting to client..!";
		exit(-1);
	}
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(port);

	if ((bind(server, (struct sockaddr*)&server_address,sizeof(server_address))) < 0) {
        cout << "Error Binding" << endl;
		exit(-1);
    }
	if(listen(server, BACKLOG) < 0) {
		cout<<"Unable to listen to port"<<endl;
		exit(-1);
	}
	/* Zero select FD sets */
	FD_ZERO(&master_list);
    FD_ZERO(&watch_list);
	/* Register the listening socket */
 	FD_SET(server, &master_list);
 	/* Register STDIN */
 	FD_SET(STDIN, &master_list);
 	head_socket = server;


	while(1) {

		memcpy(&watch_list, &master_list, sizeof(master_list));
		select_return = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
		if(select_return < 0)
            perror("select failed.");

        /* Check if we have sockets/STDIN to process */
        if(select_return > 0){
            /* Loop through socket descriptors to check which ones are ready */
            for(sock_index=0; sock_index<=head_socket; sock_index+=1){
                if(FD_ISSET(sock_index, &watch_list)){
                    /* Check if new command on STDIN */
                    if (sock_index == STDIN){
                    	char *cmd = (char*) malloc(sizeof(char)*BUFFER_SIZE);

                    	memset(cmd, '\0', BUFFER_SIZE);
						if(fgets(cmd, BUFFER_SIZE-1, stdin) == NULL)
							exit(-1);
						string command_str = cmd;
						command_str.erase(command_str.length() - 1);
						if (is_upper_string(command_str)) {
							//Input Commands
							common_commands(command_str);
							if(command_str == "LIST") {
								success_log("LIST");
								printf(printList(head).c_str());
								end_log("LIST");
							}
							if(command_str == "STATISTICS") {
								success_log("STATISTICS");
								printf(statistics(head).c_str());
								end_log("STATISTICS");
							}
						}
						else {
							cout << "Input is not all caps" << '\n';
						}

						free(cmd);
                    }
                    /* Check if new client is requesting connection */
                    else if(sock_index == server){

						addrlen = sizeof(client_address);

                        accept_fd = accept(server, (struct sockaddr *)&client_address, &addrlen);
                        if(accept_fd < 0)
                            perror("Accept failed.");


						//printf("\nRemote Host connected!\n");

						string ip = get_client_ip(sock_index,client_address);
						string dns = get_domain_from_ip(client_address);
						int port = ntohs(client_address.sin_port);

						struct Node *new_node = newNode(port, ip, dns, accept_fd);
						if (searchNode(head,ip) == NULL) {
							sortedInsert(&head,new_node);
						}
						else{
							login(head,ip);
						}

						string *a = new string[101];
						int cnt = get_count(head,ip);
						if (cnt != 0 && cnt != -1) {
							for(int k = 0; k < cnt; k++) {
								a[k] = get_msg(head,ip,k);
							}
							int j = 0;
							while(a[j] != "") {
								char *messg = (char*)a[j].c_str();
								if(send(sockIndex(head,ip), messg, strlen(messg), 0) == strlen(messg))
									printf("1Done!\n");
								fflush(stdout);
								j++;

							}
						}


						/* Add to watched socket list */
                        FD_SET(accept_fd, &master_list);
                        if(accept_fd > head_socket) head_socket = accept_fd;
                    }
                    /* Read from existing clients */
                    else{
                        /* Initialize buffer to receieve response */
                        char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
                        memset(buffer, '\0', BUFFER_SIZE);
						string ip = get_client_ip(sock_index,client_address);
                        if(recv(sock_index, buffer, BUFFER_SIZE, 0) <= 0){

							head = deleteNode(head, ip);
							close(sock_index);
							/* Remove from watched list */
                            FD_CLR(sock_index, &master_list);
                        }
                        else {
                        	//Process incoming data from existing clients here ...

							string command_str = buffer;
							command_str.erase(command_str.length() - 1);

							if (command_str.find("SEND") != string::npos) {
								string to_ip = "";
								to_ip = splitter(command_str," ",1);
								string message = "";
								message = splitterv2(command_str," ",1);
								struct Node *old_node = searchNode(head,to_ip);

								sprintf(buffer,"[%s:SUCCESS]\nmsg from:%s\n[msg]:%s\n[%s:END]\n","RECEIVED\0", (char*)(to_ip+"\0").c_str(), (char*)(message+"\0").c_str(),"RECEIVED\0");
								//buffer = (char*)(printList(head)+(string)buffer).c_str();
								if(old_node != NULL) {

									if(islogged(head,to_ip)) {

										if(send(sockIndex(head,to_ip), buffer, strlen(buffer), 0) == strlen(buffer))
											printf("2Done!\n");
										fflush(stdout);

										update_chat(head,ip,"SENT");
										update_chat(head,to_ip,"RECEIVED");

										success_log("RELAYED");

										printf("msg from:%s, to:%s\n[msg]:%s\n", (char*)(ip+"\0").c_str(), (char*)(to_ip+"\0").c_str(), (char*)(message+"\0").c_str());

										end_log("RELAYED");
									}
									else {
										storeMessage(head,to_ip,buffer);
									}

								}
							}

							if (command_str.find("BROADCAST") != string::npos) {

								string message = "";
								message = splitterv2(command_str," ",0);

								//buffer = (char*)(printList(head)+(string)buffer).c_str();

								string *ip_list = new string[4];
								ip_list[3] == "";

								int z = 0;

								ip_list[0] = get_ip_by_index(head, ip, 0);
								update_chat(head,ip,"SENT");
								while(ip_list[z] != "") {
									sprintf(buffer,"[%s:SUCCESS]\nmsg from:%s\n[msg]:%s\n[%s:END]\n","RECEIVED\0", (char*)(ip+"\0").c_str(), (char*)(message+"\0").c_str(),"RECEIVED\0");

									if(islogged(head,ip_list[z])) {

										if(send(sockIndex(head,ip_list[z]), buffer, strlen(buffer), 0) == strlen(buffer))
											printf("3Done!\n");
										fflush(stdout);


										update_chat(head,ip_list[z],"RECEIVED");

										success_log("RELAYED");

										printf("msg from:%s, to:%s\n[msg]:%s\n", (char*)(ip+"\0").c_str(), "255.255.255.255\0", (char*)(message+"\0").c_str());

										end_log("RELAYED");
									}
									else {
										storeMessage(head,ip_list[z],buffer);
									}
									z++;
									if (z != 3) {
										ip_list[z] = get_ip_by_index(head, ip, z);
									}
								}

							}

							if (command_str == "LIST") {
								buffer = (char*)printList(head).c_str();

							}
							if (command_str == "REFRESH") {
								buffer = (char*)printList(head).c_str();

							}
							if(command_str == "LOGOUT") {
								close(sock_index);
								logout(head,ip);
								FD_CLR(sock_index, &master_list);
							}

							if(command_str == "EXIT") {
								head = deleteNode(head, ip);
								close(sock_index);
								/* Remove from watched list */
	                            FD_CLR(sock_index, &master_list);
							}
							if(send(sock_index, buffer, strlen(buffer), 0) == strlen(buffer))
								printf("4Done!\n");
							fflush(stdout);
                        }
                    }
                }
            }
        }
	}
	return server;
}


//Client Side Code
int connect_to_host(const char *server_ip, int server_port) {
	int fdsocket, len;
	struct sockaddr_in remote_server_addr, client_address;

	fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(fdsocket < 0)
	   perror("Failed to create socket");

   	bzero(&client_address, sizeof(client_address));

	client_address.sin_family = AF_INET;
	client_address.sin_port = htons(port_value);

	if ((bind(fdsocket, (struct sockaddr*)&client_address,sizeof(client_address))) < 0) {
	   	cout << "Errors Binding" << endl;
		exit(-1);
	}

	bzero(&remote_server_addr, sizeof(remote_server_addr));
	remote_server_addr.sin_family = AF_INET;
	inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
	remote_server_addr.sin_port = htons(server_port);

	if(connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) < 0) {
		perror("Connect failed");
		bzero(&remote_server_addr, sizeof(remote_server_addr));
		bzero(&client_address, sizeof(client_address));
		return -1;
	}
	return fdsocket;
}

int client_program(int port) {
	port_value = port;
	int client = 0;
	int server = 1;
	bool logged = false;
	bool logged_success = false;
	string command_str;

	char *buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
	memset(buffer, '\0', BUFFER_SIZE);

	fd_set master_list;
	fd_set watch_list;
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	FD_SET(STDIN,&master_list);


	while(1) {
		memcpy(&watch_list, &master_list, sizeof(master_list));
		if (select(server+1,&watch_list,NULL,NULL,NULL) < 0){
			perror("Select Failed");
			exit(-1);
		}
		for(int i = 0; i < server+1; i++) {
			if(FD_ISSET(i, &watch_list)) {
				if (i == server) {
					if(recv(server, buffer, BUFFER_SIZE, 0) > 0){
						printf(buffer);
						success_log("LOGIN");
						end_log("LOGIN");
					} else {
						close(server);
						FD_CLR(server, &master_list);
					}
					fflush(stdout);
				}
				else if (i == STDIN) {

					if(fgets(buffer, BUFFER_SIZE-1, stdin) == NULL) {exit(-1);}

					command_str = buffer;
					command_str.erase(command_str.length() - 1);

					if (command_str.find("LOGIN") != string::npos) {
						if(command_str == "LOGIN") {
							error_log("LOGIN");
							end_log("LOGIN");
						}
						else if(server == 1) {
							string ip = "";
							int server_port = atoi(splitter(command_str," ",2).c_str());
							ip = splitter(command_str," ",1);
							if(ip_validity(ip) && server_port != 0) {
								server = connect_to_host(ip.c_str(), server_port);
								if (server == -1) {
									error_log("LOGIN");
									end_log("LOGIN");
									server = 1;
								}
								else {
									logged = true;
									FD_SET(server,&master_list);
								}

							}
							else {
								error_log("LOGIN");
								end_log("LOGIN");
							}
						}
						else {
							error_log("LOGIN");
							end_log("LOGIN");
						}

						if (logged == true && logged_success == false) {
							success_log("LOGIN");
							end_log("LOGIN");
							logged_success = true;
						}
					}
					else if(!common_commands(command_str)) {
						if(command_str.find("SEND") != string::npos) {
							string ip = splitter(command_str," ",1);
							if (!ip_validity(ip)) {
								error_log("SEND");
								end_log("SEND");
								break;
							}
						}
						if(send(server, (char*)(command_str+"\n").c_str(), strlen((char*)(command_str+"\n").c_str()), 0) == strlen((char*)(command_str+"\n").c_str()))
							printf("5Done!\n");
						fflush(stdout);

						if(recv(server, buffer, BUFFER_SIZE, 0) >= 0){
							if(command_str == "LIST" || command_str == "REFRESH") {
								success_log(command_str);
								printf(buffer);
								end_log(command_str);
							}

						}
						fflush(stdout);

						if(command_str.find("SEND") != string::npos) {
								success_log("SEND");
								end_log("SEND");
						}

						if(command_str == "LOGOUT") {
							if(logged) {
								close(server);
								logged = false;
								logged_success = false;
								FD_CLR(server, &master_list);
								server = 1;

								success_log(command_str);
								end_log(command_str);
							}
							else {
								error_log("LOGOUT");
								end_log("LOGOUT");
							}
						}
						if(command_str == "EXIT") {
							close(server);
							logged = false;
							success_log(command_str);
							end_log(command_str);
							exit(1);

						}
					}
				}
			}
		}
	}
	return client;
}


//Main Function
int main(int argc, char **argv) {

	if(argc != 3) {
		cout<<"Usage: "<<argv[0]<<" [c/s] [port]"<<endl;
		exit(-1);
	}

	if(argv[1][0] == 's') {
		server_program(atoi(argv[2]));
	}
	else if(argv[1][0] == 'c') {
		client_program(atoi(argv[2]));
	}
	return 0;
}

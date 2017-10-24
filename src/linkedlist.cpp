#include<stdlib.h>
#include<iostream>
#include<string>
#include <unistd.h>
#include <cstdlib>
#include<stdio.h>
#include <cstring>
#include <sstream>

using namespace std;

string int_to_str(int a) {
	stringstream ss;
	ss << a;
	string str = ss.str();
	return str;
}

struct Node
{
    string ip;
	int sock_index;
    bool logged;
    string hostname;
    int sent,received;
	int port;
	int message_count;
	string *messages;
	struct Node* next;
};

struct Node *newNode(int new_port, string ip, string hostname, int sock) {
	struct Node* new_node = new Node;

	new_node->port = new_port;
    new_node->ip = ip;
    new_node->hostname = hostname;
    new_node->sent = 0;
    new_node->received = 0;
    new_node->logged = true;
	new_node->message_count = 0;
	new_node->messages = new string[101];
	new_node->messages[0] = "";
	new_node->sock_index = sock;
    new_node->next = NULL;

	return new_node;
}

struct Node *searchNode(struct Node *head, string ip) {
   struct Node *ptr = NULL;
   while (head) {
      if (head->ip == ip) {
         ptr = head;
         break;
      }
      head = head->next;
   }
   return ptr;
}

void storeMessage(struct Node *head, string ip, string message) {
	struct Node *temp = searchNode(head, ip);
    if (temp == NULL) {
        cout<<"Element Not Found"<<endl;
    }
    else {
        temp->messages[temp->message_count++] = message;
		temp->messages[temp->message_count] = "";
    }
}

string relayMessage(struct Node *head, string ip) {
	struct Node *temp = searchNode(head, ip);
	if (temp == NULL) {
        cout<<"Element Not Found"<<endl;
    }
	else {
		temp->message_count = temp->message_count - 1;
		string msg = temp->messages[temp->message_count+1];
		temp->messages[temp->message_count+1] = "";
		return msg;
	}
}


void update_chat(struct Node *head, string ip, string who) {
    struct Node *temp = searchNode(head, ip);
    if (temp == NULL) {
        cout<<"Element Not Found"<<endl;
    }
    else {
        if (who == "SENT") {
            temp->sent = temp->sent + 1;
        }
        else if (who == "RECEIVED") {
            temp->received = temp->received + 1;
        }
    }
}

struct Node *logout(struct Node *head, string ip) {
    struct Node *temp = searchNode(head, ip);
    if (temp == NULL) {
        cout<<"Element Not Found"<<endl;
    }
    else {
        temp->logged = false;
    }
    return head;
}

void login(struct Node *head, string ip) {
	struct Node *temp = searchNode(head, ip);
	temp->logged = true;
}

void sortedInsert(struct Node** head_ref, struct Node* new_node) {
	struct Node* current;
	if (*head_ref == NULL || (*head_ref)->port >= new_node->port) {
		new_node->next = *head_ref;
		*head_ref = new_node;
	}
	else {
		current = *head_ref;
		while (current->next!=NULL && current->next->port < new_node->port) {
			current = current->next;
		}
		new_node->next = current->next;
		current->next = new_node;
	}
}

struct Node *deleteNode(struct Node *head, string ip) {
   struct Node *temp = searchNode(head, ip);
   if (temp == NULL) {
      cout << "Node to be deleted not found ... " << endl;
   }
   else {
      if (temp == head) {
         head = head->next;
         delete temp;
      }
      else {
         struct Node *ptr = head;
         while (ptr->next != temp) {
            ptr = ptr->next;
         }
         ptr->next = temp->next;
         delete temp;
      }
   }
   return head;
}

string printList(struct Node *head) {
	struct Node *temp = head;
    string str = "";
    int count = 1;
	while(temp != NULL)
	{
		if(temp->logged) {
			char x[300];
			sprintf(x,"%-5d%-35s%-20s%-8d\n", count, (char*)(temp->hostname+"\0").c_str(), (char*)(temp->ip+"\0").c_str(), temp->port);
	        str += x;
			count++;
		}
		temp = temp->next;
	}
    return str;
}

string statistics(struct Node *head) {
	struct Node *temp = head;
    string str = "";
    int count = 1;
	while(temp != NULL) {
		char x[300];
		string status;
		if(temp->logged) {
			status = "logged-in";
		}
		else {
			status = "logged-out";
		}
		sprintf(x,"%-5d%-35s%-8d%-8d%-8s\n", count, (char*)(temp->hostname+"\0").c_str(), temp->sent, temp->received, (char*)(status+"\0").c_str());
        str += x;
		temp = temp->next;
		count++;
	}
    return str;
}

bool islogged(struct Node *head,string ip) {
	struct Node *temp = searchNode(head, ip);
	if(temp != NULL) {
		return temp->logged;
	}
	else {
		return false;
	}
}

int sockIndex(struct Node *head, string ip) {
	struct Node *temp = searchNode(head, ip);
	if(temp != NULL) {
		return temp->sock_index;
	} else {
		return -1;
	}
}


int get_count(struct Node *head, string ip) {
   struct Node *ptr = NULL;
   while (head) {
      if (head->ip == ip) {
         ptr = head;
         break;
      }
      head = head->next;
   }
   if(ptr != NULL) {
	   return ptr->message_count;
   }
   return -1;
}

string get_msg(struct Node *head, string ip,int count) {
   struct Node *ptr = NULL;
   while (head) {
      if (head->ip == ip) {
         ptr = head;
         break;
      }
      head = head->next;
   }
   if(ptr != NULL) {
		if (count <= ptr->message_count) {
		   return ptr->messages[count];
		}
		return "";
	}
	return "";
}

string get_ip_by_index(struct Node *head, string from_ip, int index) {
	int i = 0;
	while(head) {
		if (i == index && head->ip != from_ip) {
			return head->ip;
		}
		if(head->ip != from_ip) {
			i++;
		}
		head = head->next;
	}
	return "";
}

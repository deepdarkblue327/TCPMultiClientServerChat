struct Node *searchNode(struct Node *head, string ip);

void update_chat(struct Node *head, string ip, string who);

struct Node *logout(struct Node *head, string ip);

void sortedInsert(struct Node** head_ref, struct Node* new_node);

struct Node *deleteNode(struct Node *head, string ip);

struct Node *newNode(int new_port, string ip, string hostname, int sock);

string printList(struct Node *head);

string statistics(struct Node *head);

void login(struct Node *head, string ip);

void storeMessage(struct Node *head, string ip, string message);

string relayMessage(struct Node *head, string ip);

bool islogged(struct Node *head,string ip);

int sockIndex(struct Node *head, string ip);

int get_count(struct Node *head, string ip);

string get_msg(struct Node *head, string ip,int count);

string get_ip_by_index(struct Node *head, string from_ip, int index);

bool block_client(struct Node* head, string my_ip, string client_ip);

bool unblock_client(struct Node* head, string my_ip, string client_ip);

int find_block_index(struct Node* node, string client_ip);

bool is_blocked(struct Node* head, string my_ip, string client_ip);

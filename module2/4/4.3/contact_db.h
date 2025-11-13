#ifndef CONTACT_DB_H
#define CONTACT_DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PHONE_NUMBERS 2
#define MAX_EMAIL_ADDRESSES 2
#define SOCIALS_LEN 64
#define NAME_LEN 32
#define JOB_LEN 128
#define PHONE_LEN 16
#define EMAIL_LEN 48

typedef struct {
    char telegram[SOCIALS_LEN];
    char vk[SOCIALS_LEN];
    char max[SOCIALS_LEN];
} SocialsData;

typedef struct {
    int id;
    char first_name[NAME_LEN];
    char last_name[NAME_LEN];
    char patronymic[NAME_LEN];
    char job[JOB_LEN];
    char job_position[JOB_LEN];
    char phone_numbers[MAX_PHONE_NUMBERS][PHONE_LEN];
    char email_addresses[MAX_EMAIL_ADDRESSES][EMAIL_LEN];
    SocialsData socials;
} ContactData;

typedef struct Node {
    int height;
    struct Node *left;
    struct Node *right;
    ContactData data; // key is data.id
} Node;

typedef struct {
    Node *root;
} ContactDB;

ContactDB initContactDatabase();
void destroyContactDatabase(ContactDB db);
void copyContact(ContactData *dest, const ContactData src);
ContactData *findContactByID(ContactDB db, int id);
ContactData *saveContact(ContactDB *db, ContactData contact);
int deleteContactByID(ContactDB *db, int id);
void printDB(FILE *out, ContactDB db);

int getHeight(Node *node);
int getBalance(Node *node);
int maxHeight(Node *node1, Node *node2);
Node *findMinValueNode(Node *root);
Node *createNode(int id);
void destroyNode(Node *node);
void destroyTree(Node *root);
Node *findNode(Node *root, int id);
Node *insertNode(Node *root, int id);
Node *deleteNode(Node *root, int id);
Node *rotateRight(Node *rotateNode);
Node *rotateLeft(Node *rotateNode);
void printTree(FILE *out, Node *node, int level);

#endif
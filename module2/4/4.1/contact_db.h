#ifndef CONTACT_DB_H
#define CONTACT_DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PHONE_NUMBERS 4
#define MAX_EMAIL_ADDRESSES 4
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
    Node *next;
    Node *prev;
    ContactData data; // order is based on data.id
} Node;

typedef struct {
    int size;
    int capacity;
    ContactData *contacts;
} ContactDB;

ContactDB initContactDatabase(int capacity);
void destroyContactDatabase(ContactDB db);
void copyContact(ContactData *dest, const ContactData src);
ContactData *findContactByID(ContactDB db, int id);
ContactData *saveContact(ContactDB *db, ContactData contact);
int deleteContactByID(ContactDB *db, int id);
void dumpDB(FILE *out, ContactDB db);

#endif
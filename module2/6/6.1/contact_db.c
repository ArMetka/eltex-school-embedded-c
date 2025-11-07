#include "./contact_db.h"

ContactDB initContactDatabase() {
    ContactDB db = {
        NULL,
        NULL
    };

    return db;
}

void destroyContactDatabase(ContactDB db) {
    while (db.head) {
        Node *tmp = db.head;
        db.head = db.head->next;
        free(tmp);
    }
}

void copyContact(ContactData *dest, const ContactData src) {
    dest->id = src.id;
    strncpy(dest->first_name, src.first_name, NAME_LEN);
    strncpy(dest->last_name, src.last_name, NAME_LEN);
    strncpy(dest->patronymic, src.patronymic, NAME_LEN);
    strncpy(dest->job, src.job, JOB_LEN);
    strncpy(dest->job_position, src.job_position, JOB_LEN);
    for (int i = 0; i < MAX_PHONE_NUMBERS; i++) {
        strncpy(dest->phone_numbers[i], src.phone_numbers[i], PHONE_LEN);
    }
    for (int i = 0; i < MAX_EMAIL_ADDRESSES; i++) {
        strncpy(dest->email_addresses[i], src.email_addresses[i], EMAIL_LEN);
    }
    strncpy(dest->socials.telegram, src.socials.telegram, SOCIALS_LEN);
    strncpy(dest->socials.vk, src.socials.vk, SOCIALS_LEN);
    strncpy(dest->socials.max, src.socials.max, SOCIALS_LEN);
}

ContactData *findContactByID(ContactDB db, int id) {
    Node *tmp = db.head;

    while (tmp) {
        if (tmp->data.id == id) {
            return &(tmp->data);
        }
        tmp = tmp->next;
    }

    return NULL;
}

ContactData *saveContact(ContactDB *db, ContactData contact) {
    if (!db || contact.id < 0) {
        return NULL;
    }

    ContactData *old = findContactByID(*db, contact.id);
    if (old) { // editing old contact
        copyContact(old, contact);
        return old;
    } else {   // saving new contact
        Node *new_node = (Node*)calloc(1, sizeof(Node));
        copyContact(&(new_node->data), contact);

        if (db->head == NULL) {
            db->head = new_node;
            db->tail = new_node;
            new_node->next = NULL;
            new_node->prev = NULL;
        } else {
            Node *tmp = db->head;
            Node *prev = NULL;
            while (tmp && tmp->data.id <= new_node->data.id) {
                prev = tmp;
                tmp = tmp->next;
            }

            if (prev == NULL) { // new_node->data.id is the lowest
                tmp->prev = new_node;
                new_node->next = tmp;
                new_node->prev = NULL;
                db->head = new_node;
            } else if (tmp == NULL) { // new_node->data.id is the highest
                prev->next = new_node;
                new_node->prev = prev;
                new_node->next = NULL;
                db->tail = new_node;
            } else { // somewhere inbetween
                new_node->next = tmp;
                tmp->prev = new_node;
                new_node->prev = prev;
                prev->next = new_node;
            }
        }

        return &(new_node->data);
    }
}

int deleteContactByID(ContactDB *db, int id) {
    if (!db || id < 0) {
        return 0;
    }
    
    Node *tmp = db->head;
    while (tmp && tmp->data.id != id) {
        tmp = tmp->next;
    }

    if (!tmp) {
        return 0;
    }

    if (tmp == db->head) {
        db->head = tmp->next;
        if (tmp->next) {
            tmp->next->prev = NULL;
        }
    } else if (tmp == db->tail) {
        db->tail = tmp->prev;
        if (tmp->prev) {
            tmp->prev->next = NULL;
        }
    } else {
        tmp->next->prev = tmp->prev;
        tmp->prev->next = tmp->next;
    }
    free(tmp);

    return 1;
}

void printDB(FILE *out, ContactDB db) {
    fprintf(out, "ContactDB: \n");

    Node *tmp = db.head;
    fprintf(out, "(head)\t->");
    while (tmp) {
        fprintf(out, "\t%d\t->", tmp->data.id);
        tmp = tmp->next;
    }
    fprintf(out, "\tNULL\n");

    tmp = db.head;
    fprintf(out, "NULL\t<-");
    while (tmp) {
        fprintf(out, "\t%d\t<-", tmp->data.id);
        tmp = tmp->next;
    }
    fprintf(out, "\t(tail)\n");
}

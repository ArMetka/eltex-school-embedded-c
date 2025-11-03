#include "./contact_db.h"

ContactDB initContactDatabase(int capacity) {
    ContactData *contacts = (ContactData *)calloc(capacity, sizeof(ContactData));

    for (int i = 0; i < capacity; i++) {
        (contacts + i)->id = -1;  // set id = -1 to the entire array
    }

    ContactDB db = {0, capacity, contacts};

    return db;
}

void destroyContactDatabase(ContactDB db) {
    if (db.contacts) {
        free(db.contacts);
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
    if (!db.contacts) {
        return NULL;
    }

    for (int i = 0; i < db.capacity; i++) {
        if (db.contacts[i].id == id) {
            return db.contacts + i;
        }
    }

    return NULL;
}

ContactData *saveContact(ContactDB *db, ContactData contact) {
    if (!db || !db->contacts || contact.id < 0) {
        return NULL;
    }

    ContactData *old = findContactByID(*db, contact.id);
    if (old) {  // editing old contact
        copyContact(old, contact);
        return old;
    } else if (db->size < db->capacity - 1) {         // saving new contact
        ContactData *new = findContactByID(*db, -1);  // find free
        copyContact(new, contact);
        db->size++;
        return new;
    } else {  // not enough space
        return NULL;
    }
}

int deleteContactByID(ContactDB *db, int id) {
    if (!db || !db->contacts || id < 0) {
        return 0;
    }

    ContactData *del = findContactByID(*db, id);
    if (del) {  // deleting
        memset(del, 0, sizeof(ContactData));
        del->id = -1;  // mark free
        db->size--;
        return 1;
    } else {  // not found
        return 0;
    }
}

void dumpDB(FILE *out, ContactDB db) {
    fprintf(out, "ContactDB dump\n");
    for (int i = 0; i < db.capacity; i++) {
        fprintf(out, "entry #%d:\n", i);
        fprintf(out, "1\t%d\n", db.contacts[i].id);
        fprintf(out, "2\t%s\n", db.contacts[i].first_name);
        fprintf(out, "3\t%s\n", db.contacts[i].last_name);
        fprintf(out, "4\t%s\n", db.contacts[i].patronymic);
        fprintf(out, "5\t%s\n", db.contacts[i].job);
        fprintf(out, "6\t%s\n", db.contacts[i].job_position);
        for (int j = 0; j < MAX_PHONE_NUMBERS; j++) {
            fprintf(out, "7\t%d\t%s\n", j + 1, db.contacts[i].phone_numbers[j]);
        }
        for (int j = 0; j < MAX_PHONE_NUMBERS; j++) {
            fprintf(out, "8\t%d\t%s\n", j + 1, db.contacts[i].email_addresses[j]);
        }
        fprintf(out, "9\t1\t%s\n", db.contacts[i].socials.telegram);
        fprintf(out, "9\t2\t%s\n", db.contacts[i].socials.vk);
        fprintf(out, "9\t3\t%s\n", db.contacts[i].socials.max);
        fprintf(out, "\n");
    }
}

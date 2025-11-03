#include <stdio.h>
#include <string.h>

#include "./contact_db.h"

char charp;  // char for scanf

int getID() {
    int result = 0;

    do {
        printf("enter id (>= 0): ");
        scanf("%d%c", &result, &charp);
        if (result < 0) {
            printf("invalid input\n");
        }
    } while (result < 0);

    return result;
}

void getNewContactParam(const char *prompt, char *dest, int dest_len, int is_mandatory) {
    char buf[256];
    buf[0] = '\0';

    if (is_mandatory) {
        do {
            if (*buf == '\n') {
                printf("this field can't be empty\n");
            }
            printf("%s: ", prompt);
            fgets(buf, 256, stdin);
        } while (strchr(buf, '\n') == buf);  // while first char is \n
    } else {
        printf("%s: ", prompt);
        fgets(buf, 256, stdin);
    }
    *(strchr(buf, '\n')) = '\0';
    strncpy(dest, buf, dest_len);
}

void getExistingContactParam(const char *prompt, char *dest, int dest_len, int is_mandatory) {
    char buf[256];
    buf[0] = '\0';

    printf("%s (%s): ", prompt, (*dest == '\0') ? "empty" : dest);
    fgets(buf, 256, stdin);
    char *end = strchr(buf, '\n');
    if (end != buf) {
        *end = '\0';
        strncpy(dest, buf, dest_len);
    }
}

void getContactInfo(ContactData *contact, int is_new_contact, int require_name) {
    void (*getContactParam)(const char *, char *, int, int);
    getContactParam = (is_new_contact) ? getNewContactParam : getExistingContactParam;

    if (is_new_contact) {
        memset(contact, 0, sizeof(ContactData));
    }

    printf("Enter contact info (Enter to skip (leave as is)):\n");
    getContactParam("first_name", contact->first_name, NAME_LEN, require_name);
    getContactParam("last_name", contact->last_name, NAME_LEN, require_name);
    getContactParam("patronymic", contact->patronymic, NAME_LEN, 0);
    getContactParam("job", contact->job, JOB_LEN, 0);
    getContactParam("job_position", contact->job_position, JOB_LEN, 0);
    printf("phone_numbers:\n");
    for (int i = 0; i < MAX_PHONE_NUMBERS; i++) {
        char prompt[3] = "\ti";
        prompt[1] = (char)('0' + i + 1);  // will break if i >= 10 :)
        getContactParam(prompt, contact->phone_numbers[i], PHONE_LEN, 0);
    }
    printf("email_addresses:\n");
    for (int i = 0; i < MAX_EMAIL_ADDRESSES; i++) {
        char prompt[3] = "\ti";
        prompt[1] = (char)('0' + i + 1);  // will break if i >= 10 :)
        getContactParam(prompt, contact->email_addresses[i], EMAIL_LEN, 0);
    }
    printf("socials:\n");
    getContactParam("\ttelegram", contact->socials.telegram, SOCIALS_LEN, 0);
    getContactParam("\tvk", contact->socials.vk, SOCIALS_LEN, 0);
    getContactParam("\tmax", contact->socials.max, SOCIALS_LEN, 0);
}

int printMenu() {
    int result = 0;

    printf("\n----ContactDB CLI----\n");

    printf("1) Create new contact\n");
    printf("2) Edit existing contact\n");
    printf("3) Delete contact\n");
    printf("4) Print contact\n");
    printf("5) Clear DB\n");
    printf("6) Print DB (list structure)\n");
    printf("0) Exit\n");

    printf("\nselect op: ");
    scanf("%d%c", &result, &charp);

    return result;
}

void printContact(ContactData contact) {
    printf("id: %d\n", contact.id);
    printf("first_name: %s\n", (strlen(contact.first_name) > 0) ? contact.first_name : "(empty)");
    printf("last_name: %s\n", (strlen(contact.last_name) > 0) ? contact.last_name : "(empty)");
    printf("patronymic: %s\n", (strlen(contact.patronymic) > 0) ? contact.patronymic : "(empty)");
    printf("job: %s\n", (strlen(contact.job) > 0) ? contact.job : "(empty)");
    printf("job_position: %s\n", (strlen(contact.job_position) > 0) ? contact.job_position : "(empty)");
    printf("phone_numbers:\n");
    int phones = 0;
    for (int i = 0; i < MAX_PHONE_NUMBERS; i++) {
        if (strlen(contact.phone_numbers[i]) > 0) {
            printf("\t%s\n", contact.phone_numbers[i]);
            phones++;
        }
    }
    if (!phones) {
        printf("\t(empty)\n");
    }
    printf("email_adresses:\n");
    int emails = 0;
    for (int i = 0; i < MAX_EMAIL_ADDRESSES; i++) {
        if (strlen(contact.email_addresses[i]) > 0) {
            printf("\t%s\n", contact.email_addresses[i]);
            emails++;
        }
    }
    if (!emails) {
        printf("\t(empty)\n");
    }
    printf("socials:\n");
    printf("\ttelegram: %s\n", (strlen(contact.socials.telegram) > 0) ? contact.socials.telegram : "(empty)");
    printf("\tvk: %s\n", (strlen(contact.socials.vk) > 0) ? contact.socials.vk : "(empty)");
    printf("\tmax: %s\n", (strlen(contact.socials.max) > 0) ? contact.socials.max : "(empty)");
}

int main() {
    ContactDB db = initContactDatabase();

    while (1 != 0) {
        int op = printMenu();
        if (op == 0) {  // exit
            break;
        }

        int id;
        ContactData *contact_ptr;
        ContactData contact;
        switch (op) {
            case 1:  // create
                id = -1;
                do {
                    if (id != -1) {
                        printf("Contact with this id already exists!\n");
                    }
                    id = getID();
                } while (findContactByID(db, id) != NULL);
                getContactInfo(&contact, 1, 1);
                contact.id = id;
                contact_ptr = saveContact(&db, contact);
                if (contact_ptr) {
                    printf("Successfully created, id = %d\n", contact_ptr->id);
                } else {
                    printf("Failed to create new contact\n");
                }
                break;
            case 2:  // edit
                id = getID();
                contact_ptr = findContactByID(db, id);
                if (!contact_ptr) {
                    printf("Contact not found!\n");
                    break;
                }
                getContactInfo(contact_ptr, 0, 1);
                saveContact(&db, *contact_ptr);
                break;
            case 3:  // delete
                id = getID();
                if (deleteContactByID(&db, id)) {
                    printf("Successfully deleted!\n");
                } else {
                    printf("Contact not found!\n");
                }
                break;
            case 4:  // print
                id = getID();
                contact_ptr = findContactByID(db, id);
                if (contact_ptr) {
                    printContact(*contact_ptr);
                } else {
                    printf("Contact not found!\n");
                }
                break;
            case 5:  // clear db
                destroyContactDatabase(db);
                db = initContactDatabase();
                break;
            case 6:  // dump db
                printDB(stdout, db);
                break;
            default:
                printf("Invalid input!\n");
        }
    }

    destroyContactDatabase(db);

    return 0;
}

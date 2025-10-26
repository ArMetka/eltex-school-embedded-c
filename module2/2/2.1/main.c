#include <stdio.h>
#include <string.h>

#include "./contact_db.h"

#define DB_CAPACITY 4

int generateID() {
    static int id = 0;

    return id++;
}

int getID() {
    int result = 0;

    do {
        printf("enter id (>= 0): ");
        scanf("%d", &result);
        if (result < 0) {
            printf("invalid input\n");
        }
    } while (result < 0);

    return result;
}

ContactData getContactInfo(int ignore_mandatory_fields) {
    ContactData result;

    char buf[256];
    printf("enter contact info (Enter to skip):\n");
    printf("first_name: ");
    fgets(buf, 256, stdin);

    return result;
}

int printMenu() {
    int result = 0;

    printf("\n----ContactDB CLI----\n");

    printf("1) Create new contact\n");
    printf("2) Edit existing contact\n");
    printf("3) Delete contact\n");
    printf("4) Print contact\n");
    printf("5) Clear DB\n");
    printf("6) Dump DB\n");
    printf("0) Exit\n");

    printf("\nselect op: ");
    scanf("%d", &result);

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
    ContactDB db = initContactDatabase(DB_CAPACITY);

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

                break;
            case 2:  // edit

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
                db = initContactDatabase(DB_CAPACITY);
                break;
            case 6:  // dump db
                dumpDB(stdout, db);
                break;
            default:
                printf("Invalid input!\n");
        }
    }

    destroyContactDatabase(db);

    return 0;
}

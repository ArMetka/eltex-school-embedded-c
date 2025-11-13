#include "./contact_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ContactDB initContactDatabase() {
    ContactDB db = {
        NULL
    };

    return db;
}

void destroyContactDatabase(ContactDB db) {
    destroyTree(db.root);
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
    Node *node = findNode(db.root, id);
    
    return (node) ? &node->data : NULL;
}

ContactData *saveContact(ContactDB *db, ContactData contact) {
    if (!db || contact.id < 0) {
        return NULL;
    }

    Node *old = findNode(db->root, contact.id);
    if (old) { // editing old contact
        copyContact(&old->data, contact);
        return &old->data;
    } else {   // saving new contact
        db->root = insertNode(db->root, contact.id);
        ContactData *data = &findNode(db->root, contact.id)->data;
        copyContact(data, contact);
        return data;
    }
}

int deleteContactByID(ContactDB *db, int id) {
    if (!db || id < 0) {
        return 0;
    }
    
    Node *node = findNode(db->root, id);
    if (!node) {
        return 0;
    }

    db->root = deleteNode(db->root, id);

    return 1;
}

void printDB(FILE *out, ContactDB db) {
    printTree(out, db.root, 0);
}

int getHeight(Node *node) {
    if (!node) {
        return 0;
    }
    
    return node->height;
}

int getBalance(Node *node) {
    if (!node) {
        return 0;
    }

    return getHeight(node->left) - getHeight(node->right);
}

int maxHeight(Node *node1, Node *node2) {
    return (getHeight(node1) > getHeight(node2)) ? getHeight(node1) : getHeight(node2);
}

Node *findMinValueNode(Node *root) {
    if (!root) {
        return root;
    }

    while (root->left) {
        root = root->left;
    }

    return root;
}

Node *createNode(int id) {
    Node *newnode = (Node*)calloc(1, sizeof(Node));

    newnode->height = 1;
    newnode->data.id = id;
    newnode->left = NULL;
    newnode->right = NULL;

    return newnode;
}

void destroyNode(Node *node) {
    free(node);
}

void destroyTree(Node *root) {
    if (!root) {
        return;
    }

    destroyNode(root->right);
    destroyNode(root->left);
}

Node *findNode(Node *root, int id) {
    if (!root) {
        return NULL;
    }

    if (id > root->data.id) {
        return findNode(root->right, id);
    } else if (id < root->data.id) {
        return findNode(root->left, id);
    } else {
        return root;
    }
}

Node *insertNode(Node *root, int id) {
    if (!root) {
        return createNode(id);
    }

    if (id < root->data.id) {
        root->left = insertNode(root->left, id);
    } else {
        root->right = insertNode(root->right, id);
    }
    root->height = maxHeight(root->left, root->right) + 1;

    int bal = getBalance(root);
    if (bal > 1 && id < root->left->data.id) {
        return rotateRight(root);
    }

    if (bal < -1 && id > root->right->data.id) {
        return rotateLeft(root);
    }

    if (bal > 1 && id > root->left->data.id) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (bal < -1 && id < root->right->data.id) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

Node *deleteNode(Node *root, int id) {
    if (!root) {
        return root;
    }

    if (id < root->data.id) {
        root->left = deleteNode(root->left, id);
    } else if (id > root->data.id) {
        root->right = deleteNode(root->right, id);
    } else {
        if ((!root->left) || (!root->right)) {
            Node *tmp = (root->left) ? root->left : root->right;

            if (!tmp) {
                tmp = root;
                root = NULL;
            } else {
                *root = *tmp;
            }

            destroyNode(tmp);
        } else {
            Node *tmp = findMinValueNode(root->right);
            root->data = tmp->data;
            root->right = deleteNode(root->right, tmp->data.id);
        }
    }

    if (!root) {
        return root;
    }

    root->height = maxHeight(root->left, root->right) + 1;

    int bal = getBalance(root);
    if (bal > 1 && getBalance(root->left) >= 0) {
        return rotateRight(root);
    }

    if (bal > 1 && getBalance(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    if (bal < -1 && getBalance(root->right) <= 0) {
        return rotateLeft(root);
    }

    if (bal < -1 && getBalance(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }
    
    return root;
}

Node *rotateRight(Node *rotateNode) {
    Node *left = rotateNode->left;
    Node *right_of_left = left->right;

    left->right = rotateNode;
    rotateNode->left = right_of_left;

    rotateNode->height = maxHeight(rotateNode->left, rotateNode->right) + 1;
    left->height = maxHeight(left->left, left->right) + 1;

    return left;
}

Node *rotateLeft(Node *rotateNode) {
    Node *right = rotateNode->right;
    Node *left_of_right = right->left;

    right->left = rotateNode;
    rotateNode->right = left_of_right;

    rotateNode->height = maxHeight(rotateNode->left, rotateNode->right) + 1;
    right->height = maxHeight(right->left, right->right) + 1;

    return right;
}

void printTree(FILE *out, Node *node, int level) {
    if (!node) {
        return;
    }

    printTree(out, node->right, level + 1);
    for (int i = 0; i < level; i++) {
        fputs("    ", out);
    }
    fprintf(out, "%d\n", node->data.id);

    printTree(out, node->left, level + 1);
}

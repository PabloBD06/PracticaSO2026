#include "dynamicList.h"

void createEmptyList(tList *L) {
    *L = LNULL;
}

bool isEmptyList(tList L) {
    return L == LNULL;
}

tPosL first(tList L) {
    return L;
}

tPosL last(tList L) {
    if (isEmptyList(L)) return LNULL;
    tPosL p = L;
    while (p->next != LNULL) {
        p = p->next;
    }
    return p;
}

tPosL next(tPosL p, tList L) {
    (void)L; /* Kept for interface compatibility */
    if (p == LNULL) return LNULL;
    return p->next;
}

tPosL previous(tPosL p, tList L) {
    if (p == L || p == LNULL || isEmptyList(L)) return LNULL;
    tPosL q = L;
    while (q != LNULL && q->next != p) {
        q = q->next;
    }
    return q;
}

bool insertItem(OpenFile d, tPosL p, tList *L) {
    tPosL q = (tPosL)malloc(sizeof(struct tNode));
    if (q == NULL) return false;

    q->data = d;
    q->next = LNULL;

    if (isEmptyList(*L)) {
        *L = q;
    } else if (p == LNULL) {
        /* Append at the end of the list */
        tPosL r = last(*L);
        r->next = q;
    } else if (p == *L) {
        /* Insert at the beginning of the list */
        q->next = *L;
        *L = q;
    } else {
        /* Insert before position p */
        tPosL prev = previous(p, *L);
        if (prev != LNULL) {
            prev->next = q;
            q->next = p;
        } else {
            /* Position p was not found in the list */
            free(q);
            return false;
        }
    }
    return true;
}

void deleteAtPosition(tPosL p, tList *L) {
    if (p == LNULL || isEmptyList(*L)) return;

    if (p == *L) {
        *L = (*L)->next;
        free(p);
    } else {
        tPosL prev = previous(p, *L);
        if (prev != LNULL) {
            prev->next = p->next;
            free(p);
        }
    }
}

OpenFile getItem(tPosL p, tList L) {
    (void)L;
    if (p == LNULL) return NULL;
    return p->data;
}

void updateItem(OpenFile d, tPosL p, tList *L) {
    (void)L;
    if (p != LNULL) {
        p->data = d;
    }
}

tPosL findItem(OpenFile d, tList L) {
    for (tPosL p = L; p != LNULL; p = p->next) {
        if (p->data == d) {
            return p;
        }
    }
    return LNULL;
}

void deleteList(tList *L) {
    deleteListWithData(L, NULL);
}

void deleteListWithData(tList *L, void (*freeData)(void *)) {
    tPosL p = *L;
    while (p != LNULL) {
        tPosL nextNode = p->next;
        if (freeData != NULL && p->data != NULL) {
            freeData(p->data);
        }
        free(p);
        p = nextNode;
    }
    *L = LNULL;
}

int countList(tList L) {
    int count = 0;
    for (tPosL p = L; p != LNULL; p = p->next) {
        count++;
    }
    return count;
}

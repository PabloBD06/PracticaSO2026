#ifndef DYNAMIC_LIST_H
#define DYNAMIC_LIST_H

#include <stdbool.h>
#include <stdlib.h>

#define LNULL NULL

/**
 * Generic item type for list elements.
 * Storing a pointer (void *) allows the dynamic list to hold arbitrary data structures
 * (e.g., open file entries, command history strings, memory blocks, background jobs).
 */
typedef void *tItemL;

/**
 * Position type, pointing to a node in the list.
 */
typedef struct tNode *tPosL;

struct tNode {
    tItemL data;
    tPosL next;
};

/**
 * List type, represented by a pointer to the first node.
 */
typedef tPosL tList;

/* ─── Standard TAD List Operations ─── */

/**
 * Initializes an empty list.
 * @param L Pointer to the list to initialize.
 */
void createEmptyList(tList *L);

/**
 * Checks whether the list is empty.
 * @param L The list to check.
 * @return true if the list contains no elements, false otherwise.
 */
bool isEmptyList(tList L);

/**
 * Returns the position of the first element in the list.
 * @param L The list.
 * @return Position of the first element, or LNULL if the list is empty.
 */
tPosL first(tList L);

/**
 * Returns the position of the last element in the list.
 * @param L The list.
 * @return Position of the last element, or LNULL if the list is empty.
 */
tPosL last(tList L);

/**
 * Returns the position of the element following position p.
 * @param p A valid position in the list.
 * @param L The list (kept for TAD interface compatibility).
 * @return The next position, or LNULL if p is the last element.
 */
tPosL next(tPosL p, tList L);

/**
 * Returns the position of the element preceding position p.
 * @param p A valid position in the list.
 * @param L The list.
 * @return The previous position, or LNULL if p is the first element.
 */
tPosL previous(tPosL p, tList L);

/**
 * Inserts an element into the list.
 * If p is LNULL, the item is inserted at the end of the list.
 * Otherwise, the item is inserted immediately before position p.
 * @param d The item to insert.
 * @param p The target position, or LNULL for insertion at the end.
 * @param L Pointer to the list.
 * @return true if the item was successfully inserted, false if memory allocation failed.
 */
bool insertItem(tItemL d, tPosL p, tList *L);

/**
 * Deletes the element at position p from the list.
 * Note: If the item data was dynamically allocated on the heap, caller should
 * free it before or use deleteListWithData() when destroying the list.
 * @param p The position of the element to delete.
 * @param L Pointer to the list.
 */
void deleteAtPosition(tPosL p, tList *L);

/**
 * Retrieves the item stored at position p.
 * @param p A valid position in the list.
 * @param L The list.
 * @return The item stored at position p.
 */
tItemL getItem(tPosL p, tList L);

/**
 * Updates the item stored at position p with a new value.
 * @param d The new item value.
 * @param p A valid position in the list.
 * @param L Pointer to the list.
 */
void updateItem(tItemL d, tPosL p, tList *L);

/**
 * Finds the first occurrence of an item in the list comparing pointer values.
 * @param d The item to search for.
 * @param L The list.
 * @return Position of the element if found, or LNULL otherwise.
 */
tPosL findItem(tItemL d, tList L);

/**
 * Deletes all nodes in the list and resets it to an empty list.
 * Does not free the item payloads.
 * @param L Pointer to the list.
 */
void deleteList(tList *L);

/* ─── Extended Utility Operations ─── */

/**
 * Deletes all nodes in the list, invoking a custom free callback on each element's payload.
 * Helps prevent memory leaks when elements stored in the list were allocated with malloc().
 * @param L Pointer to the list.
 * @param freeData Function pointer used to free each item's payload (e.g. free). Can be NULL.
 */
void deleteListWithData(tList *L, void (*freeData)(void *));

/**
 * Returns the number of elements currently in the list.
 * @param L The list.
 * @return Total number of elements.
 */
int countList(tList L);

#endif /* DYNAMIC_LIST_H */

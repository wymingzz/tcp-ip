#include "nlist.h"

bool nlist_is_empty(const nlist_t* list)
{
	return list->count == 0;
}

void nlist_insert_first(nlist_t* list, nlist_node_t* node)
{
	if (nlist_is_empty(list)) {
		list->first = list->last = node;
		list->count++;
		return;
	}

	list->first->pre = node;
	node->next = list->first;
	list->first = node;
	list->count++;
}

void nlist_insert_last(nlist_t* list, nlist_node_t* node)
{
	if (nlist_is_empty(list)) {
		list->first = list->last = node;
		list->count++;
		return;
	}

	list->last->next = node;
	node->pre = list->last;
	list->last = node;
	list->count++;
}

void nlist_insert_after(nlist_t* list, nlist_node_t* pre, nlist_node_t* node)
{
	if (pre == nullptr) {
		nlist_insert_first(list, node);
		return;
	}
	else if (pre->next == nullptr) {
		nlist_insert_last(list, node);
		return;
	}

	node->next = pre->next;
	pre->next = node;
	node->pre = pre;
	node->next->pre = node;
	list->count++;
}

nlist_node_t* nlist_remove(nlist_t* list, nlist_node_t* node)
{
	if (node->pre != nullptr) {
		node->pre->next = node->next;
	}
	else {
		list->first = node->next;
	}
	if (node->next != nullptr) {
		node->next->pre = node->pre;
	}
	else {
		list->last = node->pre;
	}
	node->pre = node->next = nullptr;

	list->count--;
	return node;
}

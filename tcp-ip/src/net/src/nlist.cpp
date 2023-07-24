#include "nlist.h"

void nlist_node_init(nlist_node_t* node)
{
	node->pre = node->next = nullptr;
}

void nlist_node_set_next(nlist_node_t* node, nlist_node_t* next)
{
	if (node->next != nullptr) {
		node->next->pre = next;
	}
	next->next = node->next;
	node->next = next;
	next->pre = node;
}

void nlist_node_set_pre(nlist_node_t* node, nlist_node_t* pre)
{
	if (node->pre != nullptr) {
		node->pre->next = pre;
	}
	pre->pre = node->pre;
	node->pre = pre;
	pre->next = node;
}

void nlist_init(nlist_t* list)
{
	list->first = list->last = nullptr;
	list->count = 0;
}

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

void nlist_insert_list(nlist_t* list, nlist_t* next_list, bool add_front)
{
	if (next_list == nullptr || nlist_is_empty(next_list)) {
		return;
	}
	if (nlist_is_empty(list)) {
		list->first = next_list->first;
		list->last = next_list->last;
	}
	else if (add_front) {
		list->first->pre = next_list->last;
		next_list->last->next = list->first;
		list->first = next_list->first;
	}
	else {
		list->last->next = next_list->first;
		next_list->first->pre = list->last;
		list->last = next_list->last;
	}
	list->count += next_list->count;
	nlist_init(next_list);
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

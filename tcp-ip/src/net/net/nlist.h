#ifndef NLIST_H
#define NLIST_H

typedef struct _nlist_node_t {
	_nlist_node_t* pre, * next;
	_nlist_node_t() :pre(nullptr), next(nullptr) {};
}nlist_node_t;

typedef struct _nlist_t {
	nlist_node_t* first, * last;
	int count;
	_nlist_t() :first(nullptr), last(nullptr), count(0) {};
}nlist_t;

void nlist_node_init(nlist_node_t* node);

void nlist_init(nlist_t* list);

bool nlist_is_empty(const nlist_t* list);

void nlist_insert_first(nlist_t* list, nlist_node_t* node);

void nlist_insert_last(nlist_t* list, nlist_node_t* node);

void nlist_insert_after(nlist_t* list, nlist_node_t* pre, nlist_node_t* node);

nlist_node_t* nlist_remove(nlist_t* list, nlist_node_t* node);

static inline nlist_node_t* nlist_remove_first(nlist_t* list) {
	nlist_node_t* first = list->first;
	if (first != nullptr) {
		return nlist_remove(list, first);
	}
	return nullptr;
}

#define nlist_for_each(node, list) for((node) = (list)->first; (node) != nullptr; (node) = (node)->next)

// ����node��parent_type�е�ƫ����
#define noffset_in_parent(parent_type, node_name) ((char*)(&(((parent_type*)0)) -> node_name))
// ͨ��parent_type��node�ĵ�ַ������parent_type�ĵ�ַ
#define noffset_to_parent(node, parent_type, node_name) (parent_type*)((char*)(node) - noffset_in_parent(parent_type, node_name))
// ��ֹnodeΪnullptr�����
#define nlist_entry(node, parent_type, node_name) ((node) != nullptr ? noffset_to_parent((node), parent_type, node_name) : nullptr)

#endif // !NLIS
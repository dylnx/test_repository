#ifndef LINKLIST_H_CBO2PBFV
#define LINKLIST_H_CBO2PBFV

typedef struct list *list;
typedef struct listnode *listnode;

struct listnode
{
	struct listnode *next;
	struct listnode *prev;
	void *data;
};

struct list
{
	struct listnode *head;
	struct listnode *tail;
	unsigned int count;
	int (*cmp)(void *val1, void *val2);
	void (*del)(void *val);
};

#define nextnode(X) ((X) = (X)->next)
#define listhead(X) ((X)->head)
#define listtail(X) ((X)->tail)
#define listcount(X) ((X)->count)
#define list_isempty(X) ((X)->head == NULL && (X)->tail == NULL )
#define getdata(X) ((X)->data)

struct list *list_new();
void list_free(struct list*);

void listnode_add(struct list *, void *);
void listnode_add_sort(struct list *, void *);
void listnode_add_after(struct list *, struct listnode *, void *);
void listnode_delete(struct list *, void *);
struct listnode * listnode_lookup(struct list *, void *);
void * listnode_head(struct list *);

void list_delete(struct list *);
void list_delete_all_node(struct list *);

/* Delete specific node. */
void list_delete_node (list, listnode);

void list_add_node_prev (list, listnode, void *);
void list_add_node_next (list, listnode, void *);
void list_add_list (list, list);

#define FOR_EACH(E,L) \
	for ((E) = (L)->head; (E); (E) = (E)->next) \
		if ((E)->data != NULL)

#define LIST_LOOP(L,V,N) \
  for ((N) = (L)->head; (N); (N) = (N)->next) \
    if (((V) = (N)->data) != NULL)

#define LISTNODE_ADD(L,N) \
  do { \
    (N)->prev = (L)->tail; \
    if ((L)->head == NULL) \
      (L)->head = (N); \
    else \
      (L)->tail->next = (N); \
    (L)->tail = (N); \
  } while (0)

#define LISTNODE_DELETE(L,N) \
  do { \
    if ((N)->prev) \
      (N)->prev->next = (N)->next; \
    else \
      (L)->head = (N)->next; \
    if ((N)->next) \
      (N)->next->prev = (N)->prev; \
    else \
      (L)->tail = (N)->prev; \
  } while (0)

#endif /* end of include guard: LINKLIST_H_CBO2PBFV */

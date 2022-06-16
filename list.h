#ifndef List_h
#define List_h

typedef struct List List;

// Creates a List
List * createList(void);

//Returns Current Data
void * currentList(List * list);

//Current = Head
void * firstList(List * list);

//Current = Current.next
void * nextList(List * list);

//Current = Tail
void * lastList(List * list);

//Current = Current.prev
void * prevList(List * list);

//Añade un Nodo al inicio
void pushFront(List * list, const void * data);

//Añade un Nodo al final
void pushBack(List * list, const void * data);

//Añade un Nodo a continuacion del actual
void pushCurrent(List * list, const void * data);

//Elimina el primer Nodo
void * popFront(List * list);

//Elimina el último Nodo
void * popBack(List * list);

//Elimina el Nodo actual
void * popCurrent(List * list);

//Elimina todos los Nodos
void cleanList(List * list);

#endif /* List_h */
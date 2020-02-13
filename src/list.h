#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

template <typename T>
struct ListNode
{
	ListNode *next;
	T data;
};

template <typename T>
class List
{
public:
	ListNode<T> *head;
	unsigned int length;

	List(){
		this->length = 0;
		this->head = nullptr;
	}
	~List(){
		ListNode<T> *cur = nullptr;
		ListNode<T> *next = this->head;
		while (next != nullptr)
		{
			cur = next;
			next = cur->next;
			delete cur;
		}
	}
	T& add(const T &data){
		ListNode<T> *lnodep = new ListNode<T>;
		lnodep->data = data;
		lnodep->next = this->head;
		this->head = lnodep;
		this->length++;
		return lnodep->data;
	}
};

#endif

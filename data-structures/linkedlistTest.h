#include <stdio.h>
#include <stdlib.h>

typedef struct ListNode{
    int data;
    ListNode *next;
}ListNode;


ListNode* createNode(int value)
{
    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if(NULL == newNode)
    {
        perror("malloc fail");
        return NULL;
    }

    newNode->data = value;
    newNode->next = NULL;

    return newNode;
}

ListNode* createListHeadInsert(int arr[], int n)
{
    ListNode* head = NULL;
    for(int i = 0; i < n; i++)
    {
        ListNode* newhead = createNode(arr[i]);
        if(NULL == newhead) return NULL;

        newhead->next = head;
        head = newhead;
    }

    return head;
}


ListNode* createListTailInsert(int arr[], int n)
{
    ListNode *head = createNode(arr[0]);
    ListNode *tail = head;
    for(int i = 1; i < n; i++)
    {
        ListNode* newNode = createNode(arr[i]);
        tail->next = newNode;
        tail = newNode;
    } 

    return head;
}

ListNode* insertAtPosition(ListNode *head, int pos, int value){
    if(pos <= 0)
    {
        ListNode* newhead = createNode(value);
        newhead->next = head;
        head = newhead;
        return head;
    }
    ListNode* preNode = head;
    int count = 0;
    while(preNode != NULL && count < pos -1)
    {
        count++;
        preNode = preNode->next;
    }

    if(NULL == preNode)
    {
        printf("插入位置超出链表长度");
        return head;
    }

    ListNode* newNode = createNode(value);
    newNode->next = preNode->next;
    preNode->next = newNode;
    return head;
}

ListNode* deleteAtPosition(ListNode *head, int pos, int *deletedValue) 
{
    if(NULL == head) return NULL;
    if(pos == 0)
    {
        ListNode* newhead = head->next;
        *deletedValue = head->data;
        free(head);
    }

    ListNode* pre = head;
    int count = 0;
    while(NULL != pre->next && count < pos - 1)
    {
        count++;
        pre = pre->next;
    }

    if(NULL == pre->next)
    {
        printf("删除位置超出链表长度\n");
        return head;
    }

    ListNode* deleteNode = pre->next;
    pre->next = deleteNode->next;
    *deletedValue = deleteNode->data;
    free(deleteNode);
}

ListNode* deleteByValue(ListNode *head, int value)
{
    if(head == NULL ) return NULL;
    if(value == head->data)
    {
        ListNode* newhead = head->next;
        free(head);
    }

    ListNode* pre = head;
    while(NULL != pre->next && value != pre->next->data)
    {
        pre = pre->next;
    }

    if(NULL ==  pre->next)
    {
        return head;
    }

    ListNode* ToDoDeleteNode = pre->next;
    pre->next = ToDoDeleteNode->next;
    free(ToDoDeleteNode);

    return head;
}

ListNode* reverseList(ListNode* head)
{
    ListNode* prev = NULL;
    ListNode* current = head;
    ListNode* next = NULL;

    while(current != NULL)
    {
        next = current->next;
        current->next = prev;
        current = next;
        prev = current;
    }

    return prev;
}

int hascycle(ListNode* head)
{
    if (head == NULL || head->next == NULL) {
        return 0; // 空链表或只有一个节点，无环
    }
    ListNode* fast = head;
    ListNode* slow = head;

    while(fast != NULL && fast->next != NULL)
    {
        fast = fast->next->next;
        slow = slow->next;
        if(fast == slow)
        {
            return 1;
        }
    }

    return 0;
}
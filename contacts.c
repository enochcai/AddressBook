#include<stdio.h>
#include<string.h>//memset() and strcmp()
#include<stdlib.h>

//层次划分
//   通讯录业务           文件业务        业务层
//   通讯录数据操作接口   文件操作接口    接口层
//   通讯录数据结构操作   文件操作        支持层


#define INFO printf //方便变更info为空函数
#define DEBUG printf //调试
#define NAME_LENGTH  16
#define PHONE_LENGTH 32

//支持层:链表操作,list为链表头结点,item为待插入节点，插入到头节点处
//不做输入空判断,接口层处理
#define LIST_INSERT(item, list) do {\
    item->next = list;               \
    item->prev = NULL;               \
    if((list) != NULL) (list)->prev = item; /* need() else *people->prev*/ \
    (list) = item;  /*返回头节点,为了头节点可双向修改改为二级指针ppeople*/ \
} while(0)
//define do{} while(0)多行代码替换,防止不是同一个代码块的问题

//支持层：链表操作,item为要删除的节点 --TODO:参数用item和list的好处？为何不是只要一个item参数就可以
#define LIST_REMOVE(item, list) do {    \
    if(item->prev != NULL){              \
        item->prev->next = item->next;   \
    }                                   \
    if(item->next != NULL){             \
        item->next->prev = item->prev;  \
    }                                   \
    /*处理删除头结点的情况*/              \
    if(item == list){                   \
        list = item->next;              \
    }                                   \
    item->prev = item->next = NULL;     \
} while(0)

//底层数据结构,TODO:此处链表和业务数据name phone没有分离？可以分离不？讲数据结构的操作和业务进行分离
struct person{
    char name[NAME_LENGTH];//16B
    char phone[PHONE_LENGTH];//32B
    
    struct person *next;//8B
    struct person *prev;//8B
};

struct contacts{
    struct person *people;
    int count;
};

enum {
    OPER_INSERT = 1,
    OPER_PRINT,
    OPER_DELETE,
    OPER_SEARCH,
    OPER_SAVE,
    OPER_LOAD,
};

//接口层：需要做异常处理
//ppeople 二级指针，方便当需要变更头节点时LIST_INSERT双向修改
int people_insert(struct person **ppeople, struct person *ps){
    if(ps == NULL) return -1;
    LIST_INSERT(ps, *ppeople);
    return 0;
}

int people_delete(struct person **ppeople, struct person *ps){
    if(ps == NULL) return -1;
    LIST_REMOVE(ps, *ppeople);
    return 0;
}


struct person* people_search(struct person *people, char *name){
    struct person *item = NULL;
    for(item = people;item != NULL; item = item->next){
        //same then strcmp()return 0
        if(!strcmp(item->name, name)){
            break;
        }
    }
    DEBUG("debug return item:%p,item->name:%p,item->phone:%p\n",item,&item->name,&item->phone);
    return item;
}


void people_traversal(struct person *people){
    struct person *item = NULL;
    INFO("----------------------print-start---------------------\n");
    for(item = people;item != NULL; item = item->next){
        INFO("name:%s, phone:%s\n", item->name, item->phone);
    }
    INFO("----------------------print-end------------------------\n\n");
}

//业务层
int insert_entry(struct contacts *cts){
    if(cts == NULL) return -1;
    //input and check name
    struct person *p = (struct person*)malloc(sizeof(struct person));
    if(p == NULL){
        return -2;
    }
    //memset(p, 0, sizeof(struct person));//must init else ilegal point
    
    INFO("Input name:\n");
    scanf("%s", p->name);

    //input and check phone
    INFO("Input phone:\n");
    scanf("%s", p->phone);
    //add people
    if(people_insert(&cts->people, p) != 0){
       free(p);
       return -3;
    }
    cts->count++;
    INFO("Insert Success!\n");
    return 0;
}

int print_entry(struct contacts *cts){
    if(cts == NULL) return -1;
    people_traversal(cts->people);
    return 0;
}

int delete_entry(struct contacts *cts){
    if(cts == NULL) return -1;
    //delete by name
    char name[NAME_LENGTH];
    INFO("Please Input Name:\n");
    scanf("%s", name);
    
    struct person *ps = people_search(cts->people, name); 
    if(ps == NULL){
        INFO("Person Don't Exist!\n");
        return -2;
    }
    people_delete(&cts->people, ps);
    free(ps);
    INFO("Delete People Success!\n");
    return 0;
}


int search_entry(struct contacts *cts){
    if(cts == NULL){
        INFO("contacts don't exist'");
        return -1; 
    }
    //search by name
    char name[NAME_LENGTH];
    INFO("Please Input Name:\n");
    scanf("%s", name);
    
    struct person *ps = people_search(cts->people, name); 
    if(ps == NULL){
        INFO("Person Don't Exist!\n");
        return -2;
    }
    INFO("name:%s, phone:%s\n", ps->name, ps->phone);
    //free(ps);free是释放ps指向的内存.此处ps指针不是malloc new申请的堆内存，是栈内存，函数结束会自动释放
    return 0;
    
}

//保存加载文件接口层\支持层
void save_file(struct person* people, const char* filename){
    //name:xxx, phone:xxx
    FILE *fp = fopen(filename, "w");
    if(fp == NULL) return -1;
    struct person * item = NULL;
    for(item = people;item != NULL;item = item->next){
        
    }
} 

void load_file(){
}

//保存加载文件业务层
void save_entry(struct contacts *cts){}
void load_entry(struct contacts *cts){}




int main(){
    struct contacts *cts = (struct contacts *)malloc(sizeof(struct contacts));
    if(cts == NULL) return -1;
    
    memset(cts, 0, sizeof(struct contacts));//init &cts sizeof() B 's content with 0;must init if cts may not use now;
    
    while(1){
        int select = 0;
        
        INFO("\nInput Select(1-add,2-print,3-delete,4-search,5-save,6-load):\n");
        scanf("%d", &select);

        switch(select){
            case OPER_INSERT:
                insert_entry(cts);
                break;
            case OPER_PRINT:
                print_entry(cts);
                break;
            case OPER_DELETE:
                delete_entry(cts);
                break;
            case OPER_SEARCH:
                search_entry(cts);
                break;
            case OPER_SAVE:
                save_entry(cts);
                break;
            case OPER_LOAD:
                load_entry(cts);
                break;
            default:
                break;
        }
    }
    free(cts);
    return 0;
}
    

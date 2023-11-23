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
#define BUFFER_LENGTH 128
#define MINI_TOKEN_LENGTH 5
//支持层:链表操作,list为链表头结点,item为待插入节点，插入到头节点处
//不做输入空判断,接口层处理，这里list是替换，不是参数传递,list 替换为 *people
#define LIST_INSERT(item, list) do {\
    item->next = list;               \
    item->prev = NULL;               \
    if((list) != NULL) (list)->prev = item; /* need() else *people->prev*/ \
    (list) = item;  /*返回头节,为了头节点可双向修改改为二级指针ppeople*/ \
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


//scanf防止溢出接口
int safeScanf(int width, char *var){
    if(fgets(var, width, stdin) == NULL){
		return -1;
	}

	if(var[strlen(var) - 1] == '\n'){
		var[strlen(var) -1] = '\0';
	}
	return 0;
}

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

int people_insert_sort(struct person **ppeople, struct person *ps){
    if(*ppeople == NULL){
		LIST_INSERT(ps, *ppeople);
		return 0;
	}
	struct person * cur = *ppeople;
	struct person * pre = cur->prev;
	while(cur != NULL && strcmp(cur->name, ps->name) <= 0){//升序:找到比待插入的<或=就继续，直到找到比它大就停止
		pre = cur;
		cur = cur->next;
	}
	if(pre != NULL){
		pre->next = ps;
		ps->prev = pre;
	}
	cur->prev = ps;
	ps->next = cur;
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
    if(people_insert_sort(&cts->people, p) != 0){
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
int save_file(struct person* people, const char* filename){
    //name:xxx, phone:xxx
    FILE *fp = fopen(filename, "w");
    if(fp == NULL) return -1;
    struct person * item = NULL;
    for(item = people;item != NULL;item = item->next){
		fprintf(fp, "name: %s,telephone: %s\n", item->name, item->phone);//按格式写入文件缓存
		fflush(fp);//真正写入文件
    }
	fclose(fp);
	return 0;
}

//解析文件每行格式
int parser_token(char *buffer, int length, char *name, char *phone){
	if (buffer == NULL) return -1;
	if(length < MINI_TOKEN_LENGTH) return -2;

    int i = 0, j = 0, status = 0;
	for(i = 0;buffer[i] != ','; i++){//用状态机标记开始存入name的位置直到遇到,
		if(buffer[i] == ' '){
			status = 1;
		}else if (status == 1){
			name[j++] = buffer[i];
		}
	}
	
	status = 0;
	j = 0;
	for(;i < length; i++){
		if(buffer[i] == '\n'){
			break;
		}
		else if(buffer[i] == ' '){
			status = 1;
		}else if(status == 1){
			phone[j++] = buffer[i];
		}
	}
	INFO("file token: %s --> %s\n", name, phone);
	return 0;
}

//**people才能修改
int load_file(struct person **ppeople, int *count, const char *filename){
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) return -1;
	
	while(!feof(fp)){//feof()文件末尾返回0
		char buffer[BUFFER_LENGTH] = {0};
		fgets(buffer, BUFFER_LENGTH, fp);//读取内容到buffer字符里,'\n' include 
        int length = strlen(buffer);
        //INFO("length: %d\n", length);
		//name: xxx,telephone: 1223
		char name[NAME_LENGTH] = {0};
		char phone[PHONE_LENGTH] = {0};
		if(parser_token(buffer, strlen(buffer), name, phone) != 0){
			continue;
		}
		struct person *p = (struct person*)malloc(sizeof(struct person));
		if(p == NULL) return -2;

		memcpy(p->name, name, NAME_LENGTH);
		memcpy(p->phone, phone, PHONE_LENGTH);

		people_insert_sort(ppeople, p);
		(*count) ++;
	}
	fclose(fp);
	return 0;
}

//保存加载文件业务层
int save_entry(struct contacts *cts){
	if (cts == NULL) return -1;
	INFO("Please Input Save Filename: \n");
	char filename[NAME_LENGTH] = {0};
	scanf("%s", filename);

	save_file(cts->people, filename);
}

int load_entry(struct contacts *cts){	
	if (cts == NULL) return -1;
	INFO("Please Input Load Filename: \n");
	char filename[NAME_LENGTH] = {0};
	scanf("%s", filename);

    load_file(&cts->people, &cts->count, filename);
}


void menu_info(void) {

    INFO("\n\n********************************************************\n");
    INFO("***** 1. Add Person\t\t2. Print People ********\n");
    INFO("***** 3. Del Person\t\t4. Search Person *******\n");
    INFO("***** 5. Save People\t\t6. Load People *********\n");
    INFO("***** Other Key for Exiting Program ********************\n");
    INFO("********************************************************\n\n");

}

int main(){
    struct contacts *cts = (struct contacts *)malloc(sizeof(struct contacts));
    if(cts == NULL) return -1;
    
    memset(cts, 0, sizeof(struct contacts));//init &cts sizeof() B 's content with 0;must init if cts may not use now;
    
    while(1){
        int select = 0;
        
        menu_info();
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
                goto exit;
        }
    }
exit:
    free(cts);

    return 0;
}
    

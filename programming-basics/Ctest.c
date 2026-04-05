#include <stdio.h>
#include <stdlib.h>

/*指针
1、指针是一个存放变量地址的变量，32系统占 4 字节， 64 系统占 8字节
指针运算 + - 按类型大小跳跃，int * + 1 跳四字节
*/
void test1()
{
    int x = 10;
    // 指针变量p存放变量x的地址
    int *p = &x;

    printf("sizeof(p) = %zu bytes (64位系统)\n", sizeof(p));
    printf("x = %d, *p = %d\n", x, *p);
    printf("&x = %p, p = %p\n", &x, p);

    // 指针运算示例
    int arr[5] = {10, 20, 30, 40, 50};
    int *a = arr;  // 数组名退化指针，指向第一个元素

    printf("\n指针运算示例:\n");
    printf("a = %p, *a = %d\n", (void*)a, *a);
    printf("a + 1 = %p, *(a + 1) = %d (跳过了%ld字节)\n", (void*)(a + 1), *(a + 1), (char*)(a + 1) - (char*)a);
    printf("a + 2 = %p, *(a + 2) = %d (跳过了%ld字节)\n", (void*)(a + 2), *(a + 2), (char*)(a + 2) - (char*)a);

    // 不同类型指针步长不同
    char *char_p = (char*)arr;
    printf("\n不同类型指针步长不同:\n");
    printf("char* + 1 跳过 %ld 字节\n", (char*)(char_p + 1) - (char*)char_p);

    // malloc分配示例
    int *dynamic_arr = malloc(5 * sizeof(int));
    if (dynamic_arr != NULL) {
        for (int i = 0; i < 5; i++) {
            dynamic_arr[i] = i * 10;
        }
        printf("\nmalloc分配的数组:\n");
        for (int i = 0; i < 5; i++) {
            printf("dynamic_arr[%d] = %d\n", i, dynamic_arr[i]);
        }
        free(dynamic_arr);
    }
}

/*
指针和数组的关系
数组名是数组首元素地址，但是数组名是一个常量，不能修改
数组是一片连续的内存区域，数组名表示这块连续内存区域的首地址，会退化成指向首元素的指针常量，不可以指向其它地址
*/

void test2() {
    int arr[10] = {0};
    int *p = arr;

    // 以下几种访问方式等价
    arr[0] = 10;
    p[0] = 10;
    *(p + 0) = 10;
    *(arr + 0) = 10;

    printf("\n指针数组关系示例:\n");
    printf("arr = %p, &arr[0] = %p\n", arr, &arr[0]);
    printf("sizeof(arr) = %zu (整个数组大小)\n", sizeof(arr));
    printf("sizeof(p) = %zu (指针大小)\n", sizeof(p));
}

/*
野指针：指针指向的内容已经被释放，但指针没有置空
避免：malloc/free后及时置NULL，不指向栈上已经释放的变量
*/
int* bad_function() {
    int x = 100;  // x在栈上
    return &x;    // 返回局部变量地址，x出栈后就失效了，这就是野指针！
}
/*
const 指针和常量指针的区别
- const int *p: 常量指针（指向常量的指针），*p 无法修改，p本身可以修改
- int *const p: 指针常量（指针本身是常量），p 无法修改，*p 可以修改
口诀：const在*左是常量指针，const在*右是指针常量
*/
void test3()
{
    int a = 10;
    int b = 20;
    const int *p = &a;   // 常量指针：指向const
    int *const p2 = &a;  // 指针常量：指针本身const

    // 错误：不能通过p修改指向的值
    // *p = 12;  // ❌

    // 正确：p本身可以改变指向
    p = &b;  // ✅

    // 正确：可以通过p2修改指向的值
    *p2 = 12;  // ✅

    // 错误：p2本身不能改变指向
    // p2 = &b;  // ❌

    /*这里虽然可以用非const指针修改const的变量的值，但是会产生警告，不建议这么使用，要用const指针指向const变量*/
    const int c = 10;
    int *p3 = (int*)&c;  // 需要强转，否则编译警告
    *p3 = 12;
    printf("p now points to b = %d, c = %d (不推荐修改const变量)\n", *p, c);
}

/*
指针和字符串的关系
字符串本质上是一个以 \0 为结尾的char数组
*/
void test4()
{
    //执行字符串的一个指针，hello是被放在文字常量区的，是一个只读区域，不可修改。
    char* str = "hello";
    //下面两种用法因为修改了常量区，都会导致段错误
    //*str = 'a';
    //str[0] = 'a';
    //直接定义数组的话，这里str在栈上，相当于把hello在文字常量区的拷贝了一份。可以修改
    char str1[] = "hello";
    str1[0] = 'a';
    printf("str %s\n", str);
    printf("str1 %s\n", str1);
    printf("first char of str: %c\n", *str);  // ✅ 输出单个字符
}

/*
函数指针
函数指针就是指向函数的入口地址，允许函数当做数据来使用，存储和动态调用
返回类型(*指针名)(参数列表)
*/
typedef void (*event_handler)();
typedef struct{
    char *event_name;
    event_handler handler;
}EventMap;

// 前置声明
void callback(void(*func)());
void on_click();
void on_hover();

void on_click(){printf("click\n");}
void on_hover(){printf("Hover\n");}

void test5()
{
    void (*p)() = test3;
    p();
    callback(p);
    EventMap events[] = {{"click", on_click},
                        {"hover", on_hover}};
    events[0].handler();
}

void callback(void(*func)())
{
    func();
}



int main()
{
    //printf("=== test1: 指针基本概念 ===\n");
    //test1();

    //printf("\n=== test2: 指针和数组 ===\n");
    //test2();

    test5();
    
    return 0;
}
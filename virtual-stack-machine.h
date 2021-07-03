#include "string"
#include "map"
#include "vector"
#include "stack"
using namespace std;
enum class OP_TYPE
{
    NUMBER,
    NAME,
    ADD,
    ASSIGN
};
enum class ATOM_TYPE
{
    NAME,
    NUMBER
};
struct Atom
{
    ATOM_TYPE type;
    double fval; //TODO 删除
    long long int nval;
    string sval;
};
enum class SYMOBL_TYPE
{
    ARGUMENT,
    VARIABLE,
    PROPERTY
};
struct Symobl
{
    SYMOBL_TYPE type;
    uint8_t slot;
    Symobl *next;
    struct Entry
    {
        Atom *key;
        void *value; //datum | property
    } entry;
};
enum class DATUM_TYPE
{
    NUMBER,
    STRING,
    FUNCTION,
    ATOM,
    SYMBOL
};
struct Datum
{
    DATUM_TYPE type;
    struct
    {
        long long int nval;
        string sval;
        Atom *atom;
        Symobl *sym;
    } u;
};
struct CodeGenerator
{
    uint8_t *base; //指令的起始地址
    uint8_t *ptr;  //指令移动指针
};
struct Script
{
    vector<Atom *> atoms;
    Symobl *args;    //参数符号
    uint8_t argc;    //参数数量
    uint8_t *code;   //代码
    unsigned length; //代码长度
};
struct Object
{
    Symobl *list; //符号链
    Object *parent;
};
struct Stack
{
    Datum *base; //数据栈
    Datum *ptr;  //指向最近的数据栈
};
struct Context
{
    vector<Atom *> atoms;            //TODO 删除
    map<string, Atom *> globalScope; //TODO 删除
    Script *script;                  //根代码
    Stack stack;                     //数据栈
    Object *staticLink;              //顶层的作用域
};
CodeGenerator *NewCodeGenerator();
Script *NewScript(CodeGenerator *cg);
void excuteCode(Context *kc, Script *script, Object *staticLink, Datum *result);
void generateCode(Context *kc, CodeGenerator *cg);
Atom *getAtom(Context *kc, int index);
Atom *generateAtom(string val);
Atom *generateAtom(double val);
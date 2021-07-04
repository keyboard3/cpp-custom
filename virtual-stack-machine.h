#include "string"
#include "map"
#include "vector"
#include "stack"
using namespace std;
typedef unsigned long number;
enum class OP_TYPE
{
    NUMBER,
    NAME,
    ADD,
    ASSIGN,
    RETURN
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
    number nval;
    string sval;
};
enum class SYMBOL_TYPE
{
    ARGUMENT,
    VARIABLE,
    PROPERTY
};
struct Symbol
{
    SYMBOL_TYPE type;
    uint8_t slot;
    Symbol *next;
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
        Symbol *sym;
    } u;
};
struct CodeGenerator
{
    uint8_t *base; //指令的起始地址
    uint8_t *ptr;  //指令移动指针
};
class Script
{
public:
    Script(unsigned codeLimit)
    {
        code = (uint8_t *)malloc(sizeof(uint8_t) * codeLimit);
        args = nullptr;
        argc = 0;
        length = 0;
    }
    vector<Atom *> atoms;
    Symbol *args;    //参数符号
    uint8_t argc;    //参数数量
    uint8_t *code;   //代码
    unsigned length; //代码长度
};
class Object
{
public:
    Object()
    {
        list = nullptr;
        parent = nullptr;
    }
    Symbol *list; //符号链
    Object *parent;
};
struct Stack
{
    Datum *base; //数据栈
    Datum *ptr;  //指向最近的数据栈
};
class Context
{
public:
    Context(int stackSize)
    {
        script = new Script(1000);
        staticLink = nullptr;
        stack.ptr = stack.base = (Datum *)malloc(sizeof(Datum) * stackSize);
        staticLink = new Object();
    }
    Script *script;     //根代码
    Stack stack;        //数据栈
    Object *staticLink; //顶层的作用域
};
Symbol *findSymbol(Object *scope, Atom *atom);
void excuteCode(Context *kc, Script *script, Object *slink, Datum *result);
void generateCode(Script *script);
void pushAtom(Atom *atom, Stack *stack);
void pushNumber(number value, Stack *stack);
Datum *popDatum(Stack *stack);
bool resolveSymbol(Context *context, Datum *dp);
bool resolveValue(Context *context, Datum *dp);
Atom *getAtom(Script *script, int index);
Atom *generateAtom(string val);
Atom *generateAtom(number val);
string to_op_str(OP_TYPE op);
void dumpScope(Object *scope);
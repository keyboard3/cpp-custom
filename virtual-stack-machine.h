#include "string"
#include "map"
#include "vector"
#include "stack"
using namespace std;
typedef unsigned long number;
class Scope;
class Function;
enum class OP_TYPE
{
    NUMBER,
    NAME,
    ADD,
    ASSIGN,
    RETURN,
    CALL
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
enum class DATUM_TYPE
{
    UNDEF,
    NUMBER,
    STRING,
    FUNCTION,
    ATOM,
    SYMBOL
};
class Symbol
{
public:
    Symbol(SYMBOL_TYPE type, Scope *scope) : type(type), scope(scope)
    {
        next = nullptr;
    }
    SYMBOL_TYPE type;
    uint8_t slot;
    Symbol *next;
    Scope *scope;
    struct Entry
    {
        Atom *key;
        void *value; //datum | property
    } entry;
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
        Function *fun;
    } u;
};
class Property
{
public:
    Property(Datum *d)
    {
        datum = d;
        next = nullptr;
    }
    Datum *datum; //属性一般是直接挂到symbol上的
    Property *next;
};
class Scope
{
public:
    Scope(Scope *parent) : parent(parent)
    {
        list = nullptr;
    }
    Symbol *list;    //符号链
    Property *props; //属性链
    Scope *parent;
};
//指令集合
class Script
{
public:
    Script(unsigned codeLimit)
    {
        code = (uint8_t *)malloc(sizeof(uint8_t) * codeLimit);
        args = nullptr;
        length = 0;
    }
    vector<Atom *> atoms; //指令中引用的字面量
    Symbol *args;         //指令的形参符号（主要在函数中）
    uint8_t *code;        //代码
    unsigned length;      //代码长度
};
//函数对象存储是函数的指令集合
class Function
{
public:
    Function(Atom *name, Script *script, Scope *slink) : name(name), script(script), slink(slink) {}
    Atom *name;     //函数名
    Script *script; //指令集合
    Scope *slink;   //函数定义所在的静态作用域
};
//函数执行在栈中的具体体现
struct Frame
{
    unsigned argc;  //实际参数数量
    Datum *argv;    //参数起始地址
    unsigned nvars; //变量数量
    Datum *vars;    //变量起始地址
    Function *fun;  //当前栈帧所指向的函数
    Frame *down;    //上一个栈帧
    Scope *scope;   //当前运行栈帧所处的作用域
};
struct Stack
{
    Datum *base;  //数据栈
    Datum *ptr;   //指向最近的数据栈
    Frame *frame; //当前运行的栈帧
};
class Context
{
public:
    Context(int stackSize)
    {
        script = new Script(1000);
        staticLink = nullptr;
        stack.ptr = stack.base = (Datum *)malloc(sizeof(Datum) * stackSize);
        stack.frame = nullptr;
        staticLink = new Scope(nullptr);
    }
    Script *script;    //根代码
    Stack stack;       //数据栈
    Scope *staticLink; //顶层的作用域
};
Symbol *findSymbol(Scope *scope, Atom *atom);
void codeInterpret(Context *kc, Script *script, Scope *slink, Datum *result);
void generateCode(Context *kc);
void pushAtom(Atom *atom, Stack *stack);
void pushNumber(number value, Stack *stack);
Datum *popDatum(Stack *stack);
bool resolveSymbol(Context *context, Datum *dp);
bool resolveValue(Context *context, Datum *dp);
Atom *getAtom(Script *script, int index);
Atom *generateAtom(string val);
Atom *generateAtom(number val);
string to_op_str(OP_TYPE op);
void dumpScope(Scope *scope);
void pushSymbol(Symbol *sym, Scope *scope);
void pushDatum(Stack *stack, Datum *d);
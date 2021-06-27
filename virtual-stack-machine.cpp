#include "iostream"
#include "vector"
#include "stack"
#include "map"
using namespace std;

enum OP_TYPE
{
    NUMBER,
    NAME,
    ADD,
    ASSIGN
};
enum ATOM_TYPE
{
    ATOM_NAME,
    ATOM_NUMBER
};
struct Atom
{
    ATOM_TYPE flags;
    double fval;
    string sval;
};
struct Context
{
    vector<Atom *> atoms;
    map<string, Atom *> globalScope;
};
struct CodeGenerator
{
    uint8_t *base; //指令的起始地址
    uint8_t *ptr;  //指令移动指针
};
struct Script
{
    uint8_t *code;
    unsigned length;
};
CodeGenerator *NewCodeGenerator();
Script *NewScript(CodeGenerator *cg);
void excuteCode(Context &kc, Script *script);
void generateCode(Context &kc, CodeGenerator *cg);
Atom *getAtom(Context &kc, int index);
Atom *generateAtom(string val);
Atom *generateAtom(double val);
int main()
{
    Context kc;
    CodeGenerator *cg = NewCodeGenerator();
    generateCode(kc, cg);
    Script *script = NewScript(cg);
    excuteCode(kc, script);
}
void excuteCode(Context &kc, Script *script)
{
    uint8_t *ptr = script->code;
    uint8_t *end = ptr + script->length;
    stack<Atom *> stack;
    while (ptr < end)
    {
        OP_TYPE op = (OP_TYPE)ptr[0];
        switch (op)
        {
        case NUMBER:
            stack.push(getAtom(kc, ptr[1]));
            ptr++;
            break;
        case NAME:
        {
            Atom *nameAtom = getAtom(kc, ptr[1]);
            if (kc.globalScope.find(nameAtom->sval) == kc.globalScope.end())
                throw nameAtom->sval + " not defined";
            stack.push(kc.globalScope[nameAtom->sval]);
            ptr++;
        }
        break;
        case ASSIGN:
        {
            Atom *nameAtom = getAtom(kc, ptr[1]);
            Atom *valueAtom = stack.top();
            stack.pop();
            kc.globalScope[nameAtom->sval] = valueAtom;
            ptr++;
        }
        break;
        case ADD:
            Atom *lval = stack.top();
            stack.pop();
            Atom *rval = stack.top();
            stack.pop();
            stack.push(generateAtom(lval->fval + rval->fval));
            break;
        }
        ptr++;
    }

    map<string, Atom *>::iterator it;
    for (it = kc.globalScope.begin(); it != kc.globalScope.end(); ++it)
    {
        cout << it->first << ": ";
        if (it->second->flags == ATOM_NUMBER)
            cout << it->second->fval << endl;
        else
            cout << it->second->sval << endl;
    }
}
void emit1(CodeGenerator *cg, OP_TYPE type)
{
    cg->ptr[0] = type;
    cg->ptr++;
}
void emit2(CodeGenerator *cg, OP_TYPE type, uint8_t op1)
{
    cg->ptr[0] = type;
    cg->ptr[1] = op1;
    cg->ptr += 2;
}
void generateCode(Context &kc, CodeGenerator *cg)
{
    //构建atoms
    kc.atoms.push_back(generateAtom(2));
    kc.atoms.push_back(generateAtom(3));
    kc.atoms.push_back(generateAtom("a"));
    kc.atoms.push_back(generateAtom("c"));
    //构建指令
    emit2(cg, NUMBER, 0);
    emit2(cg, NUMBER, 1);
    emit1(cg, ADD);
    emit2(cg, ASSIGN, 2);
    emit2(cg, NAME, 2);
    emit2(cg, NUMBER, 1);
    emit1(cg, ADD);
    emit2(cg, ASSIGN, 3);
};
Atom *getAtom(Context &kc, int index)
{
    return kc.atoms[index];
}
Atom *generateAtom(double val)
{
    Atom *atom = new Atom();
    atom->flags = ATOM_NUMBER;
    atom->fval = val;
    return atom;
}
Atom *generateAtom(string val)
{
    Atom *atom = new Atom();
    atom->flags = ATOM_NAME;
    atom->sval = val;
    return atom;
}
CodeGenerator *NewCodeGenerator()
{
    CodeGenerator *cg = new CodeGenerator();
    //暂时分配1000代码缓存区
    cg->ptr = cg->base = (uint8_t *)malloc(sizeof(uint8_t) * 1000);
    return cg;
}
Script *NewScript(CodeGenerator *cg)
{
    Script *script = new Script();
    script->length = cg->ptr - cg->base;
    script->code = (uint8_t *)malloc(sizeof(uint8_t) * script->length);
    memcpy(script->code, cg->base, script->length);
    return script;
}
#include "iostream"
#include "vector"
#include "stack"
#include "map"
#include "virtual-stack-machine.h"
using namespace std;
int main()
{
    Context context;
    context.stack.ptr = context.stack.base = (Datum *)malloc(sizeof(Datum) * 1000);
    CodeGenerator *cg = NewCodeGenerator();
    generateCode(context, cg);
    Script *script = NewScript(cg);
    context.staticLink = new Object();
    Datum *result;
    excuteCode(&context, script, context.staticLink, result);
}
void excuteCode(Context *context, Script *script, Object *staticLink, Datum *result)
{
    uint8_t *ptr = script->code;
    uint8_t *end = ptr + script->length;
    Stack &stack = context->stack;
    while (ptr < end)
    {
        OP_TYPE op = (OP_TYPE)ptr[0];
        switch (op)
        {
        case OP_TYPE::NAME:
        case OP_TYPE::NUMBER:
        {
            Datum d;
            d.type = DATUM_TYPE::ATOM;
            d.u.atom = getAtom(context, ptr[1]);
            stack.ptr[0] = d;
            stack.ptr++;
        }
        break;
        case OP_TYPE::ASSIGN:
        {
            Datum *rval = stack.ptr--;
            resolveValue(context, rval);
            Datum *lval = stack.ptr--;
            resolveSymbol(context, lval);
            if (lval->type == DATUM_TYPE::ATOM)
            {
                //构建解析符号，数据挂到自己上
                Symobl *sym = new Symobl();
                sym->type = SYMOBL_TYPE::VARIABLE;
                sym->entry.key = lval->u.atom;
                sym->entry.value = rval;
                auto next = context->staticLink->list;
                sym->next = next;
                context->staticLink->list = sym;
            }
            else
            {
                //将数据覆盖到解析符号的值上
                lval->u.sym->entry.value = rval;
            }
        }
        break;
        case OP_TYPE::ADD:
            Datum *rval = stack.ptr--;
            resolveValue(context, rval);
            Datum *lval = stack.ptr--;
            resolveValue(context, lval);
            long long int value = lval->u.nval + rval->u.nval;
            //将结果压入栈中
            Datum val;
            val.u.nval = value;
            val.type = DATUM_TYPE::NUMBER;
            stack.ptr[0] = val;
            stack.ptr++;
        }
        ptr++;
    }
}
bool resolveValue(Context *context, Datum *dp)
{
    return true;
}
bool resolveSymbol(Context *context, Datum *dp)
{
    return true;
}
void emit1(CodeGenerator *cg, OP_TYPE type)
{
    cg->ptr[0] = (uint8_t)type;
    cg->ptr++;
}
void emit2(CodeGenerator *cg, OP_TYPE type, uint8_t op1)
{
    cg->ptr[0] = (uint8_t)type;
    cg->ptr[1] = op1;
    cg->ptr += 2;
}
void generateCode(Context &context, CodeGenerator *cg)
{
    //构建atoms
    context.atoms.push_back(generateAtom(2));
    context.atoms.push_back(generateAtom(3));
    context.atoms.push_back(generateAtom("a"));
    context.atoms.push_back(generateAtom("c"));
    //构建指令
    emit2(cg, OP_TYPE::NUMBER, 0);
    emit2(cg, OP_TYPE::NUMBER, 1);
    emit1(cg, OP_TYPE::ADD);
    emit2(cg, OP_TYPE::ASSIGN, 2);
    emit2(cg, OP_TYPE::NAME, 2);
    emit2(cg, OP_TYPE::NUMBER, 1);
    emit1(cg, OP_TYPE::ADD);
    emit2(cg, OP_TYPE::ASSIGN, 3);
};
Atom *getAtom(Context *context, int index)
{
    return context->atoms[index];
}
Atom *generateAtom(double val)
{
    Atom *atom = new Atom();
    atom->type = ATOM_TYPE::NUMBER;
    atom->fval = val;
    return atom;
}
Atom *generateAtom(string val)
{
    Atom *atom = new Atom();
    atom->type = ATOM_TYPE::NAME;
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
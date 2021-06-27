#include "iostream"
#include "stack"
using namespace std;

enum OP_TYPE
{
    NUMBER,
    ADD
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
void excuteCode(Script *script);
void generateCode(CodeGenerator *cg);
int main()
{
    CodeGenerator *cg = NewCodeGenerator();
    generateCode(cg);
    Script *script = NewScript(cg);
    excuteCode(script);
}
void excuteCode(Script *script)
{
    uint8_t *ptr = script->code;
    uint8_t *end = ptr + script->length;
    stack<uint8_t> stack;
    while (ptr < end)
    {
        OP_TYPE op = (OP_TYPE)ptr[0];
        switch (op)
        {
        case NUMBER:
            stack.push(ptr[1]);
            ptr++;
            break;
        case ADD:
            uint8_t lval = stack.top();
            stack.pop();
            uint8_t rval = stack.top();
            stack.pop();
            stack.push(lval + rval);
            break;
        }
        ptr++;
    }
    cout << (int)stack.top() << endl;
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
void generateCode(CodeGenerator *cg)
{
    emit2(cg, NUMBER, 2);
    emit2(cg, NUMBER, 3);
    emit1(cg, ADD);
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
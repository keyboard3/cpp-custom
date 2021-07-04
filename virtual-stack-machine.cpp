#include "iostream"
#include "vector"
#include "stack"
#include "map"
#include "virtual-stack-machine.h"
using namespace std;
int main()
{
  Context *context = new Context(1000);
  generateCode(context->script);
  Datum *result = new Datum();
  excuteCode(context, context->script, context->staticLink, result);
  cout << "result:" << result->u.nval << endl;
}
void excuteCode(Context *context, Script *script, Object *slink, Datum *result)
{
  uint8_t *ptr = script->code;
  uint8_t *end = ptr + script->length;
  Stack *stack = &context->stack;
  auto oldslink = context->staticLink;
  context->staticLink = slink;
  while (ptr < end)
  {
    OP_TYPE op = (OP_TYPE)ptr[0];
    cout << "op:" << to_op_str(op) << endl;
    ptr++;

    switch (op)
    {
    case OP_TYPE::NAME:
    case OP_TYPE::NUMBER:
      pushAtom(getAtom(script, ptr[0]), stack);
      ptr++;
      break;
    case OP_TYPE::ASSIGN:
    {
      Datum *lval = popDatum(stack);
      resolveSymbol(context, lval);
      Datum *rval = popDatum(stack);
      resolveValue(context, rval);
      if (lval->type == DATUM_TYPE::ATOM)
      {
        //构建解析符号，数据挂到自己上
        Symbol *sym = new Symbol();
        sym->type = SYMBOL_TYPE::VARIABLE;
        sym->entry.key = lval->u.atom;
        sym->entry.value = new Datum();
        memcpy(sym->entry.value, rval, sizeof(Datum));
        sym->slot = 1;

        auto next = context->staticLink->list;
        sym->next = next;
        context->staticLink->list = sym;
      }
      else
      {
        //将数据覆盖到解析符号的值上
        cout << "将数据覆盖到解析符号的值上" << lval->u.sym->entry.key->sval << endl;
        lval->u.sym->entry.value = rval;
      }
    }
    break;
    case OP_TYPE::ADD:
    {
      Datum *lval = popDatum(stack);
      resolveValue(context, lval);
      Datum *rval = popDatum(stack);
      resolveValue(context, rval);
      number value = lval->u.nval + rval->u.nval;
      //将结果压入栈中
      pushNumber(value, stack);
    }
    break;
    case OP_TYPE::RETURN:
    {
      unsigned hasVal = ptr[0];
      ptr++;
      if (hasVal != 1)
        break;
      Datum *rval = popDatum(stack);
      bool isOk = resolveValue(context, rval);
      memcpy(result, rval, sizeof(Datum));
    }
    break;
    }
  }
  context->staticLink = oldslink;
}
Datum *popDatum(Stack *stack)
{
  return --stack->ptr;
}
void pushAtom(Atom *atom, Stack *stack)
{
  Datum d;
  d.type = DATUM_TYPE::ATOM;
  d.u.atom = atom;
  stack->ptr[0] = d;
  stack->ptr++;
}
void pushNumber(number value, Stack *stack)
{
  Datum val;
  val.u.nval = value;
  val.type = DATUM_TYPE::NUMBER;
  stack->ptr[0] = val;
  stack->ptr++;
}
bool resolveValue(Context *context, Datum *dp)
{
  bool isOk = resolveSymbol(context, dp);
  auto primaryAtomToDatum = [&]()
  {
    if (dp->type == DATUM_TYPE::ATOM)
    {
      auto atom = dp->u.atom;
      if (atom->type == ATOM_TYPE::NUMBER)
      {
        dp->type = DATUM_TYPE::NUMBER;
        dp->u.nval = atom->nval;
        return true;
      }
    }
    return false;
  };
  if (dp->type == DATUM_TYPE::SYMBOL)
  {
    auto sym = dp->u.sym;
    if (sym->type == SYMBOL_TYPE::PROPERTY)
    {
      cout << "resolveValue PROPERTY" << endl;
      //TODO 处理成从propty上拿
      primaryAtomToDatum();
      return true;
    }
    if (sym->entry.value)
    {
      memcpy(dp, (Datum *)sym->entry.value, sizeof(Datum));
      //TODO 获取到的
      primaryAtomToDatum();
      return true;
    }
    cout << "准备从栈帧中获取" << endl;
    //证明这个是从根栈帧上的符号，所以要找到scope对应的栈帧
    if (sym->type == SYMBOL_TYPE::ARGUMENT)
    {
      //TODO 如果是参数，读取栈帧上的参数区域数据
    }
    else if (sym->type == SYMBOL_TYPE::VARIABLE)
    {
      //TODO 如果是变量，则读取栈帧上的变量区域数据
    }
    primaryAtomToDatum();
    return true;
  }
  else
    return primaryAtomToDatum();
  return false;
}
bool resolveSymbol(Context *context, Datum *dp)
{
  if (dp->type == DATUM_TYPE::ATOM)
  {
    auto atom = dp->u.atom;
    if (atom->type == ATOM_TYPE::NAME)
    {
      auto sym = findSymbol(context->staticLink, atom);
      if (!sym)
        return false;
      //只有name atom需要去作用域中找到是否存在已解析的符号
      dp->type = DATUM_TYPE::SYMBOL;
      dp->u.sym = sym;
      return true;
    }
  }
  else if (dp->type == DATUM_TYPE::SYMBOL)
    return true;
  return false;
}
Symbol *findSymbol(Object *scope, Atom *atom)
{
  if (scope == nullptr)
    return nullptr;
  for (Symbol *sym = scope->list; sym; sym = sym->next)
  {
    if (sym->entry.key == atom)
      return sym;
  }
  return findSymbol(scope->parent, atom);
}
void emit1(Script *script, OP_TYPE type)
{
  unsigned base = script->length;
  script->code[base + 0] = (uint8_t)type;
  script->length++;
}
void emit2(Script *script, OP_TYPE type, uint8_t op1)
{
  unsigned base = script->length;
  script->code[base + 0] = (uint8_t)type;
  script->code[base + 1] = op1;
  script->length += 2;
}
void generateCode(Script *script)
{
  //构建atoms
  script->atoms.push_back(generateAtom(2));
  script->atoms.push_back(generateAtom(3));
  script->atoms.push_back(generateAtom("a"));
  script->atoms.push_back(generateAtom("c"));
  //构建指令
  emit2(script, OP_TYPE::NUMBER, 0);
  emit2(script, OP_TYPE::NUMBER, 1);
  emit1(script, OP_TYPE::ADD);
  emit2(script, OP_TYPE::NAME, 2);
  emit1(script, OP_TYPE::ASSIGN);
  emit2(script, OP_TYPE::NAME, 2);
  emit2(script, OP_TYPE::NUMBER, 1);
  emit1(script, OP_TYPE::ADD);
  emit2(script, OP_TYPE::NAME, 3);
  emit1(script, OP_TYPE::ASSIGN);
  emit2(script, OP_TYPE::NAME, 3);
  emit2(script, OP_TYPE::RETURN, 1);
  cout << "code " << script->length << endl;
};
Atom *getAtom(Script *script, int index)
{
  return script->atoms[index];
}
Atom *generateAtom(number val)
{
  Atom *atom = new Atom();
  atom->type = ATOM_TYPE::NUMBER;
  atom->nval = val;
  return atom;
}
Atom *generateAtom(string val)
{
  Atom *atom = new Atom();
  atom->type = ATOM_TYPE::NAME;
  atom->sval = val;
  return atom;
}
void dumpScope(Object *scope)
{
  for (Symbol *sym = scope->list; sym; sym = sym->next)
  {
    Datum *temp = (Datum *)sym->entry.value;
    cout << sym->entry.key->sval << " " << temp->u.nval << endl;
  }
}
string to_op_str(OP_TYPE op)
{
  switch (op)
  {
  case OP_TYPE::NUMBER:
    return "number";
  case OP_TYPE::NAME:
    return "name";
  case OP_TYPE::ADD:
    return "add";
  case OP_TYPE::ASSIGN:
    return "assign";
  case OP_TYPE::RETURN:
    return "return";
  }
}
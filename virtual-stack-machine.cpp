#include "iostream"
#include "vector"
#include "stack"
#include "map"
#include "virtual-stack-machine.h"
using namespace std;
int main()
{
  Context *context = new Context(1000);
  generateCode(context);
  Datum *result = new Datum();
  codeInterpret(context, context->script, context->staticLink, result);
  dumpScope(context->staticLink);
}
void codeInterpret(Context *context, Script *script, Scope *slink, Datum *result)
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
        Symbol *sym = new Symbol(SYMBOL_TYPE::VARIABLE, context->staticLink);
        sym->entry.key = lval->u.atom;
        if (stack->frame != nullptr)
        {
          pushDatum(stack, rval);
          sym->slot = stack->frame->nvars;
          stack->frame->nvars++;
        }
        else
        {
          sym->entry.value = new Datum();
          memcpy(sym->entry.value, rval, sizeof(Datum));
        }
        pushSymbol(sym, context->staticLink);
      }
      else if (lval->u.sym->type == SYMBOL_TYPE::PROPERTY)
      {
        Property *prop = (Property *)lval->u.sym->entry.value;
        memcpy(prop->datum, rval, sizeof(Datum));
      }
      else if (stack->frame == nullptr)
        lval->u.sym->entry.value = rval;
      else if (lval->u.sym->type == SYMBOL_TYPE::ARGUMENT)
        memcpy(&stack->frame->argv[lval->u.sym->slot], rval, sizeof(Datum));
      else if (lval->u.sym->type == SYMBOL_TYPE::VARIABLE)
        memcpy(&stack->frame->vars[lval->u.sym->slot], rval, sizeof(Datum));
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
    case OP_TYPE::CALL:
    {
      unsigned argc = ptr[0];
      ptr++;
      Datum *funDatum = stack->ptr - argc - 1;
      bool isOK = resolveValue(context, funDatum);
      if (funDatum->type != DATUM_TYPE::FUNCTION)
        throw funDatum->u.atom->sval + " not defined";
      //创建栈帧
      Frame *frame = new Frame();
      frame->down = stack->frame;
      stack->frame = frame;
      frame->fun = funDatum->u.fun;
      frame->argc = argc;
      frame->argv = stack->ptr - argc;
      frame->nvars = 0;
      frame->vars = stack->ptr;
      frame->scope = new Scope(frame->fun->slink);
      frame->scope->list = frame->fun->script->args;
      //给参数符号初始化栈帧的位置
      int slot = 0;
      for (Symbol *sym = frame->scope->list; sym != nullptr && slot < argc; sym = sym->next, slot++)
      {
        resolveValue(context, &frame->argv[slot]); //参数需要解析成数据才能用
        sym->scope = frame->scope;
        sym->slot = slot;
      }
      //调用
      Datum *result = new Datum();
      result->type = DATUM_TYPE::UNDEF;
      codeInterpret(context, frame->fun->script, frame->scope, result);
      //调用完毕还原栈帧
      stack->frame = frame->down;
      stack->ptr = frame->argv;
      if (result->type != DATUM_TYPE::UNDEF)
        pushDatum(stack, result);
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
void pushDatum(Stack *stack, Datum *d)
{
  stack->ptr = d;
  stack->ptr++;
}
void pushSymbol(Symbol *sym, Scope *scope)
{
  auto next = scope->list;
  sym->next = next;
  scope->list = sym;
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
        // cout << "primary value:" << dp->u.nval << endl;
        return true;
      }
    }
    // else if (dp->type == DATUM_TYPE::NUMBER)
    //   cout << "primary value:" << dp->u.nval << endl;
    return false;
  };
  if (dp->type == DATUM_TYPE::SYMBOL)
  {
    auto sym = dp->u.sym;
    if (sym->type == SYMBOL_TYPE::PROPERTY)
    {
      Property *prop = (Property *)sym->entry.value;
      memcpy(dp, prop->datum, sizeof(Datum));
      primaryAtomToDatum();
      return true;
    }
    if (sym->entry.value)
    {
      memcpy(dp, (Datum *)sym->entry.value, sizeof(Datum));
      primaryAtomToDatum();
      return true;
    }
    Frame *targetFp = nullptr;
    for (Frame *fp = context->stack.frame; fp != nullptr; fp = fp->down)
    {
      if (fp->scope == sym->scope)
      {
        targetFp = fp;
        break;
      }
    }
    if (targetFp == nullptr)
    {
      cout << "未找到栈帧" << endl;
      return false;
    }
    //证明这个是从根栈帧上的符号，所以要找到scope对应的栈帧
    if (sym->type == SYMBOL_TYPE::ARGUMENT)
      memcpy(dp, &targetFp->argv[sym->slot], sizeof(Datum));
    else if (sym->type == SYMBOL_TYPE::VARIABLE)
      memcpy(dp, &targetFp->vars[sym->slot], sizeof(Datum));
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

Symbol *findSymbol(Scope *scope, Atom *atom)
{
  if (scope == nullptr)
    return nullptr;
  for (Symbol *sym = scope->list; sym != nullptr; sym = sym->next)
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
void generateCode(Context *context)
{
  Script *script = context->script;
  script->atoms.push_back(generateAtom(2));
  script->atoms.push_back(generateAtom(3));
  script->atoms.push_back(generateAtom("a"));
  script->atoms.push_back(generateAtom("add"));
  emit2(script, OP_TYPE::NUMBER, 0);
  emit2(script, OP_TYPE::NUMBER, 1);
  emit1(script, OP_TYPE::ADD);
  emit2(script, OP_TYPE::NAME, 2);
  emit1(script, OP_TYPE::ASSIGN);
  auto generateAddMethod = []() -> Script *
  {
    Script *script = new Script(1000);
    script->atoms.push_back(generateAtom(12));
    script->atoms.push_back(generateAtom(3));
    script->atoms.push_back(generateAtom("a"));
    script->atoms.push_back(generateAtom("arg"));
    //构建atoms
    emit2(script, OP_TYPE::NUMBER, 0);
    emit2(script, OP_TYPE::NAME, 3);
    emit1(script, OP_TYPE::ADD);
    emit2(script, OP_TYPE::NUMBER, 1);
    emit1(script, OP_TYPE::ADD);
    emit2(script, OP_TYPE::NAME, 2);
    emit1(script, OP_TYPE::ASSIGN);
    emit2(script, OP_TYPE::NAME, 2);
    emit2(script, OP_TYPE::RETURN, 1);
    //将代码中符号是参数的提前挂出来
    Symbol *sym = new Symbol(SYMBOL_TYPE::ARGUMENT, nullptr);
    sym->entry.key = script->atoms[3];
    script->args = sym;
    return script;
  };
  emit2(script, OP_TYPE::NAME, 3);
  emit2(script, OP_TYPE::NAME, 2);
  emit2(script, OP_TYPE::CALL, 1);
  emit2(script, OP_TYPE::NAME, 2);
  emit1(script, OP_TYPE::ASSIGN);
  //函数调用指令
  Script *mScript = generateAddMethod();
  //建立函数符号
  Symbol *sym = new Symbol(SYMBOL_TYPE::PROPERTY, context->staticLink);
  sym->entry.key = script->atoms[3];
  //创建function数据
  Datum *fund = new Datum();
  fund->type = DATUM_TYPE::FUNCTION;
  fund->u.fun = new Function(sym->entry.key, mScript, context->staticLink);
  //设置为属性的符号
  sym->entry.value = new Property(fund);
  pushSymbol(sym, context->staticLink);
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
void dumpScope(Scope *scope)
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
  case OP_TYPE::CALL:
    return "call";
  }
}
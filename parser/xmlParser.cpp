#include "list"
#include "stdlib.h"
#include "string"
using namespace std;

enum Token {
  tok_eof = -1,
  // primary 基础元素
  tok_identifier = -2,
  tok_beginTag = -4,
  tok_closeTag = -5,
};

// gettok()拿到tok类型之后，数据从这里拿
static string IdentifierStr;
static string TagStr;
static int LastChar = ' ';
static int gettok() {
  while (isspace(LastChar))
    LastChar = getchar();

  //识别出标识符: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum(LastChar = getchar()))
      IdentifierStr += LastChar;
    return tok_identifier;
  }
  // startTag or endTag
  if (LastChar == '<') {
    bool isEndTag = false;
    LastChar = getchar(); // eat <
    if (LastChar == '/') {
      isEndTag = true;
      LastChar = getchar(); // eat /
    }
    //标签后面应该是标识符
    int idType = gettok(); // eat tok_identifier
    if (idType != tok_identifier)
      throw "xml is error";
    TagStr = IdentifierStr;
    LastChar = getchar(); // eat >
    if (isEndTag)
      return tok_closeTag;
    return tok_beginTag;
  }
  if (LastChar == EOF) {
    return tok_eof;
  }

  int ThisChar = LastChar;
  LastChar = getchar(); // eat cur and ready next
  return ThisChar;
}

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

enum NodeType { node_num = 1, node_str = 2, node_list = 3 };
class Node {
public:
  Node(string strVal) : strVal(strVal) { type = node_str; }
  Node(list<pair<string, Node *>> *list) : list(list) { type = node_list; }
  NodeType type;
  string strVal;
  list<pair<string, Node *>> *list;
  void print(string tab, string key) {
    if (type == node_str) {
      printf("%s key:%s val:%s\n", tab.c_str(), key.c_str(), strVal.c_str());
      return;
    }
    printf("%s key:%s\n", tab.c_str(), key.c_str());
    for (auto it = list->begin(); it != list->end(); ++it) {
      it->second->print(tab + "\t", it->first);
    }
  }
};

void parseXml(string parentName, Node *&slot) {
  getNextToken(); // eat beginTag

  string strVal = "";
  list<pair<string, Node *>> *child = new list<pair<string, Node *>>();
  //解析标签正文内容，以识别出父节点的结束标签或者是字符结尾结束
  while (CurTok != tok_eof &&
         !(CurTok == tok_closeTag && TagStr == parentName)) {
    //如果正文内容里，识别出了标签开头
    if (CurTok == tok_beginTag) {
      Node *node = nullptr;
      //解析这个标签的内容了，以当前的TagStr为结束标记
      parseXml(TagStr, node);
      child->push_back({TagStr, node});
      continue;
    }
    //如果是结束标签（相对应了前面的开始标签）
    if (CurTok == tok_closeTag) {
      getNextToken(); // eat closeTag
      continue;
    }
    //这里就是解析纯文本内容
    if (CurTok == tok_identifier) {
      if (strVal != "")
        strVal += " ";
      strVal += IdentifierStr;
    } else
      strVal += CurTok;
    getNextToken(); // eat tok_number
  }
  if (child->size() == 0)
    slot = new Node(strVal);
  else
    slot = new Node(child);
}

int main(int argc, char *argv[]) {
  getNextToken(); // eat begin xml
  if (CurTok == tok_beginTag && TagStr == "xml") {
    Node *root = nullptr;
    parseXml("xml", root);
    getNextToken(); // eat close xml
    root->print("", "xml");
    return 0;
  }
  perror("not found xml root\n");
  return 0;
}
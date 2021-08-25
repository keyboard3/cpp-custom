#include "fstream"
#include "iostream"
#include "list"
#include "map"
#include "stdlib.h"
#include "string"
using namespace std;

enum Token {
  tok_eof = -1,
  // primary 基础元素
  tok_identifier = -2,
  tok_number = -3,
  tok_beginTag = -4,
  tok_closeTag = -5,
};

// gettok()拿到tok类型之后，数据从这里拿
static string IdentifierStr;
static string TagStr;
static double NumVal;
static int gettok() {
  static int LastChar = ' ';
  while (isspace(LastChar))
    LastChar = getchar();

  //识别出标识符: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum(LastChar = getchar()))
      IdentifierStr += LastChar;
    return tok_identifier;
  }
  //识别出数字: [0-9.]+
  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');
    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }
  // startTag or endTag
  if (LastChar == '<') {
    bool isEndTag = false;
    LastChar = getchar();
    if (LastChar == '/') {
      isEndTag = true;
      LastChar = getchar();
    }
    //标签后面应该是标识符
    int idType = gettok();
    if (idType != tok_identifier)
      throw "xml is error";
    TagStr = IdentifierStr;
    LastChar = getchar();
    if (isEndTag)
      return tok_closeTag;
    return tok_beginTag;
  }
  if (LastChar == EOF) {
    return tok_eof;
  }
  //严格xml格式不会到最后1步
  return LastChar;
}

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

enum NodeType { node_num = 1, node_str = 2, node_list = 3 };
class Node {
public:
  Node(string strVal) : strVal(strVal) { type = node_str; }
  Node(double numVal) : numVal(numVal) { type = node_num; }
  Node(list<pair<string, Node *>> *list) : list(list) { type = node_list; }
  NodeType type;
  string strVal;
  double numVal;
  list<pair<string, Node *>> *list;
  void print(string tab, string key) {
    if (type == node_str) {
      printf("%s key:%s val:%s\n", tab.c_str(), key.c_str(), strVal.c_str());
      return;
    }
    if (type == node_num) {
      printf("%s key:%s val:%f\n", tab.c_str(), key.c_str(), numVal);
      return;
    }
    printf("%s key:%s\n", tab.c_str(), key.c_str());
    for (auto it = list->begin(); it != list->end(); ++it) {
      it->second->print(tab + "\t", it->first);
    }
  }
};

void parseXml(string parentName, Node *&slot) {
  getNextToken();
  //消化掉内容及结束节点
  if (CurTok == tok_identifier) {
    slot = new Node(IdentifierStr);
    getNextToken();
    return;
  }
  if (CurTok == tok_number) {
    slot = new Node(NumVal);
    getNextToken();
    return;
  }

  list<pair<string, Node *>> *child = new list<pair<string, Node *>>();
  while (CurTok != tok_eof &&
         !(CurTok == tok_closeTag && TagStr == parentName)) {
    if (CurTok == tok_beginTag) {
      Node *node = nullptr;
      parseXml(TagStr, node);
      child->push_back({TagStr, node});
    }
    if (CurTok == tok_closeTag) {
      CurTok = getNextToken();
    }
  }
  slot = new Node(child);
}

int main(int argc, char *argv[]) {
  Node *node = new Node(2);

  getNextToken();
  if (CurTok == tok_beginTag && TagStr == "xml") {
    Node *root = nullptr;
    parseXml("xml", root);
    getNextToken();
    root->print("", "xml");
    return 0;
  }
  perror("not found xml root\n");
  return 0;
}
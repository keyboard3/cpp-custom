#include "htmlParser.h"
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
  tok_string = -4,
  tok_beginTag = -5,
  tok_closeTag = -6,
};

// gettok()拿到tok类型之后，数据从这里拿
static string gContent;      //读取的html内容
static string::iterator git; //迭代指针

static string IdentifierStr; //属性名
static string StrVal;        //属性值
static string TagStr;        //标签名
static int LastChar = ' ';
static int getStrChar() {
  if (git == gContent.end())
    return EOF;
  int cur = *git;
  git++;
  return cur;
}
static int gettok() {
  while (isspace(LastChar))
    LastChar = getStrChar();

  //识别出标识符: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum(LastChar = getStrChar()))
      IdentifierStr += LastChar;
    return tok_identifier;
  }
  if (LastChar == '"') {
    string str;
    LastChar = getStrChar(); // eat "
    do {
      str += LastChar;
      LastChar = getStrChar();
    } while (LastChar != '"');
    LastChar = getStrChar();
    StrVal = str;
    return tok_string;
  }
  // startTag or endTag
  if (LastChar == '<') {
    bool isEndTag = false;
    LastChar = getStrChar(); // eat <
    if (LastChar == '/') {
      isEndTag = true;
      LastChar = getStrChar(); // eat /
    }
    //标签后面应该是标识符
    int idType = gettok();
    if (idType != tok_identifier) {
      perror("html is error");
      return tok_eof;
    }
    TagStr = IdentifierStr;
    if (isEndTag) {
      LastChar = getStrChar(); // eat >
      return tok_closeTag;
    }
    // beigin 会多出>
    return tok_beginTag;
  }
  if (LastChar == EOF) {
    return tok_eof;
  }
  int ThisChar = LastChar;
  LastChar = getStrChar(); // ready next
  return ThisChar;
}

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

/*
  解析内容，将解析结果放到slot上
*/
void parseTag(string parentName, DOM *dom) {
  if (CurTok != tok_beginTag) {
    perror("must beigin tag");
    return;
  }
  // eat beginTag and attributes and >
  getNextToken();
  while (CurTok != '>' && CurTok != tok_eof) {
    if (CurTok != tok_identifier) {
      perror("attribute error");
      return;
    }
    string attributeName = IdentifierStr;
    getNextToken(); // eat tok_identifier
    if (CurTok != '=') {
      dom->attributes.insert({attributeName, "true"}); //默认true =1
      continue;
    }
    getNextToken(); // eat =
    if (CurTok != tok_string) {
      string errMsg =
          dom->name + " attribute " + attributeName + " missing value";
      perror(errMsg.c_str());
      return;
    }
    dom->attributes.insert({attributeName, StrVal});
    getNextToken(); // eat tok_string
  }

  if (CurTok != '>') {
    perror("tag error");
    return;
  }
  getNextToken(); // eat >

  //解释标签内容
  list<DOM *> *child = new list<DOM *>();
  string content;
  while (CurTok != tok_eof &&
         !(CurTok == tok_closeTag && TagStr == parentName)) {
    //将非javascript标签下归档处理
    if (CurTok == tok_beginTag) {
      if (parentName == "javascript") {
        content += "<" + TagStr;
        getNextToken();
        continue;
      }
      DOM *dom = new DOM(TagStr);
      parseTag(TagStr, dom);
      child->push_back(dom);
      continue;
    }
    if (CurTok == tok_closeTag) {
      if (parentName == "javascript") {
        content += "</" + TagStr + ">";
        getNextToken();
        continue;
      }
      getNextToken(); // eat closeTag
      continue;
    }
    if (CurTok == tok_string)
      content += StrVal;
    else if (CurTok == tok_identifier)
      content += IdentifierStr;
    else
      content += CurTok;
    getNextToken();
  }
  // dom->children = new Node(child);
  if (child->size() > 0)
    dom->children = new Node(child);
  else
    dom->children = new Node(content);
}

string getFileContent(string filepath) {
  string content, line;
  ifstream htmlFile(filepath);
  if (htmlFile.is_open()) {
    while (getline(htmlFile, line)) {
      content += line;
    }
    htmlFile.close();
  }
  return content;
}

DOM *parseHtml(string filpath) {
  gContent = getFileContent(filpath);
  git = gContent.begin();

  getNextToken(); // eat <html
  string rootTag = "html";
  if (CurTok == tok_beginTag && TagStr == rootTag) {
    DOM *root = new DOM(rootTag);
    parseTag(rootTag, root);
    getNextToken();
    return root;
  }
  return nullptr;
}

// int main(int argc, char *argv[]) {
//   DOM *root = getRoot("index.html");
//   if (root != nullptr)
//     root->children->print("");
//   return 0;
// }
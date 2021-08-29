#include "list"
#include "map"
#include "string"
using namespace std;

#ifndef HTML_DEFINE
#define HTML_DEFINE

enum NodeType { node_str = 2, node_list = 3 };
class Node;
class DOM {
public:
  string name;
  map<string, string> attributes;
  string innerHtml;
  Node *children;
  DOM(string name) : name(name) {}
};
class Node {
public:
  Node(string strVal) : strVal(strVal) { type = node_str; }
  Node(list<DOM *> *list) : list(list) { type = node_list; }
  NodeType type;
  string strVal;
  list<DOM *> *list;
  void print(string tab) {
    if (type == node_str) {
      printf("%s%s", tab.c_str(), strVal.c_str());
      return;
    }
    for (auto it : *list) {
      printf("%s TAG %s:", tab.c_str(), it->name.c_str());
      for (auto attriute : it->attributes) {
        printf("%s %s=%s", tab.c_str(), attriute.first.c_str(),
               attriute.second.c_str());
      }
      printf("\n");
      it->children->print(tab + "\t");
      printf("\n");
    }
  }
};
DOM *parseHtmlByPath(string filpath);
void parseHtmlToDOM(string content, DOM *dom);
#endif
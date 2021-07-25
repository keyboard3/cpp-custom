#include "string"
#include "iostream"
using namespace std;
string getPlatformEndianness();
int main()
{
  //小端序
  cout << getPlatformEndianness() << endl;
  int first = 11;
  int arr1[2] = {0, 1};
  int second = 12;
  int arr2[2] = {2, 3};
  int third = 13;
  printf("%d %d %d\n", first, *(&first - 1), *(&first - 2));
  printf("%d %d %d %d\n", arr1[0], *(&arr1[0] + 1), *(&arr1[0] - 1), *(&arr1[0] - 2));
  printf("%d\n", *(&arr2[0] - 2));
  //内存分配数组在一起连续分配，变量在一起连续分配
  //栈内存地址是从高地址->低地址的增长趋势 arr1[1],arr1[0],arr2[1],arr2[0],[],first,second,third
  //数组内存排列走向符合小端序的方向，从低地址->高地址
  return 0;
}
string getPlatformEndianness()
{
  int nNum = 0x12345678;
  char chData = *(char *)(&nNum);
  //内存的低地址存放的数据的高位
  if (chData == 0x12)
    return "大端序";
  //内存低地址存放数据的低位
  else
    return "小端序";
}
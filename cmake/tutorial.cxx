// 一个计算数字平方根的简单程序
#include "./MathFunctions/MathFunctions.h"
#include "TutorialConfig.h"
#include <cmath>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  //没参数，就打印程序的版本
  if (argc < 2) {
    // argv[0]是程序名称，包含了程序所在的路径
    std::cout << argv[0] << " Version " << Tutorial_VERSION_MAJOR << "."
              << Tutorial_VERSION_MINOR << std::endl;
    std::cout << "Usage: " << argv[0] << " number" << std::endl;
    return 1;
  }

  //有参数就将输入转成double c++11的特性(string)
  const double inputValue = std::stod(argv[1]);

  const double outputValue = mathfunctions::sqrt(inputValue);
  std::cout << "The square root of " << inputValue << " is " << outputValue
            << std::endl;
  return 0;
}
#include "RateLimiterModule.h"
#include <folly/Optional.h>
#include <iostream>

int main() {
  std::cout << "Hello, World!" << std::endl;
  folly::Optional<int> i{};
  i.emplace(1);
  std::cout << i.value() << std::endl;

  return 0;
}

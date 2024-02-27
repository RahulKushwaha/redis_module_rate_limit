//
// Created by Rahul  Kushwaha on 2/27/24.
//
#pragma once
#include <folly/TokenBucket.h>

namespace rk::projects::redis_rate_limiter {

class RateLimiter {
 public:
  explicit RateLimiter();

  bool consume(double toConsume);

  double balance();

 private:
  std::shared_ptr<folly::TokenBucket> tokenBucket_;
};

}  // namespace rk::projects::redis_rate_limiter

#ifdef __cplusplus
extern "C" {
#endif
typedef void* RateLimiterHandle;

RateLimiterHandle createRateLimiter() {
  return (rk::projects::redis_rate_limiter::RateLimiter*)new rk::projects::
      redis_rate_limiter::RateLimiter();
}

void freeRateLimiter(RateLimiterHandle handle) {
  delete (rk::projects::redis_rate_limiter::RateLimiter*)handle;
}

bool consume(RateLimiterHandle handle, double toConsume) {
  return ((rk::projects::redis_rate_limiter::RateLimiter*)handle)
      ->consume(toConsume);
}

double balance(RateLimiterHandle handle) {
  return ((rk::projects::redis_rate_limiter::RateLimiter*)handle)->balance();
}

#ifdef __cplusplus
}
#endif

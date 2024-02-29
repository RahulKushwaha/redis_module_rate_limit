//
// Created by Rahul  Kushwaha on 2/27/24.
//

#include "RateLimiter.h"

#include "glog/logging.h"

namespace rk::projects::redis_rate_limiter {

RateLimiter::RateLimiter(double genRate, double burstSize)
    : tokenBucket_{std::make_shared<folly::TokenBucket>(genRate, burstSize)} {};

bool RateLimiter::consume(double toConsume) {
  auto result = tokenBucket_->consume(toConsume);
  return result;
}

double RateLimiter::balance() {
  return tokenBucket_->balance();
}

void RateLimiter::reset(double genRate, double burstSize) {
  tokenBucket_->reset(genRate, burstSize);
}

#ifdef __cplusplus
extern "C" {
#endif
RateLimiterHandle createRateLimiter() {
  return (rk::projects::redis_rate_limiter::RateLimiter*)new rk::projects::
      redis_rate_limiter::RateLimiter(1, 1);
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

void reset(RateLimiterHandle handle, double genRate, double burstSize) {
  ((rk::projects::redis_rate_limiter::RateLimiter*)handle)
      ->reset(genRate, burstSize);
}
}
}  // namespace rk::projects::redis_rate_limiter
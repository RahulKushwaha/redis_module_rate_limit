//
// Created by Rahul  Kushwaha on 2/27/24.
//

#include "RateLimiter.h"

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

}  // namespace rk::projects::redis_rate_limiter
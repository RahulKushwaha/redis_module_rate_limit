//
// Created by Rahul  Kushwaha on 2/27/24.
//

#include "RateLimiter.h"

namespace rk::projects::redis_rate_limiter {

RateLimiter::RateLimiter()
    : tokenBucket_{std::make_shared<folly::TokenBucket>(1, 1)} {};

bool RateLimiter::consume(double toConsume) {
  auto result = tokenBucket_->consume(toConsume);
  return result;
}

double RateLimiter::balance() {
  return tokenBucket_->balance();
}

}  // namespace rk::projects::redis_rate_limiter
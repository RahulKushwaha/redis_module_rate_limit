//
// Created by Rahul  Kushwaha on 2/27/24.
//
#pragma once
#include "folly/TokenBucket.h"

namespace rk::projects::redis_rate_limiter {

class RateLimiter {
 public:
  explicit RateLimiter(double genRate, double burstSize);

  bool consume(double toConsume);

  double balance();

  void reset(double genRate, double burstSize);

 private:
  std::shared_ptr<folly::TokenBucket> tokenBucket_;
};

}  // namespace rk::projects::redis_rate_limiter

#ifdef __cplusplus
extern "C" {
#endif
typedef void* RateLimiterHandle;

RateLimiterHandle createRateLimiter();
void freeRateLimiter(RateLimiterHandle handle);
bool consume(RateLimiterHandle handle, double toConsume);
double balance(RateLimiterHandle handle);
void reset(RateLimiterHandle handle, double genRate, double burstSize);

#ifdef __cplusplus
}
#endif

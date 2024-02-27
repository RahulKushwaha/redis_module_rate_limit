//
// Created by Rahul  Kushwaha on 2/26/24.
//

#include "/Users/rahulkushwaha/projects/redis/src/redismodule.h"
#include "RateLimiter.h"
#include "folly/Optional.h"
#include <iostream>

#include <strings.h>

static RateLimiterHandle rate_limiter_handle;

static RedisModuleString *log_key_name;

static const char ratelimiter_block_command_name[] = "ratelimiter.block";
static int in_log_command = 0;

unsigned long long unfiltered_clientid = 0;

static RedisModuleCommandFilter *filter;
static RedisModuleString *retained;

int CommandFilter_PingCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModule_Log(ctx, "notice", "exceeded quota");
  RedisModule_ReplyWithError(ctx, "TOO_MANY_REQUESTS");

  return REDISMODULE_OK;
}

void CommandFilter_CommandFilter(RedisModuleCommandFilterCtx *filterCtx) {
  unsigned long long id = RedisModule_CommandFilterGetClientId(filterCtx);
  if (id == unfiltered_clientid) return;

  if (in_log_command) return; /* don't process our own RM_Call() from CommandFilter_LogCommand() */

  if (!consume(rate_limiter_handle, 1)) {
    RedisModule_CommandFilterArgInsert(filterCtx, 0,
                                       RedisModule_CreateString(nullptr, ratelimiter_block_command_name, sizeof(ratelimiter_block_command_name) - 1));
  }
}

extern "C" int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (RedisModule_Init(ctx, "commandfilter", 1, REDISMODULE_APIVER_1)
      == REDISMODULE_ERR) return REDISMODULE_ERR;

  if (argc != 2 && argc != 3) {
    RedisModule_Log(ctx, "warning", "Log key name not specified");
    return REDISMODULE_ERR;
  }

  rate_limiter_handle = createRateLimiter();

  long long noself = 0;
  log_key_name = RedisModule_CreateStringFromString(ctx, argv[0]);
  RedisModule_StringToLongLong(argv[1], &noself);
  retained = nullptr;

  if (RedisModule_CreateCommand(ctx, ratelimiter_block_command_name,
                                CommandFilter_PingCommand, "deny-oom", 1, 1, 1)
      == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  if ((filter = RedisModule_RegisterCommandFilter(ctx, CommandFilter_CommandFilter,
                                                  noself ? REDISMODULE_CMDFILTER_NOSELF : 0))
      == nullptr) return REDISMODULE_ERR;

  if (argc == 3) {
    const char *ptr = RedisModule_StringPtrLen(argv[2], nullptr);
    if (!strcasecmp(ptr, "noload")) {
      /* This is a hint that we return ERR at the last moment of OnLoad. */
      RedisModule_FreeString(ctx, log_key_name);
      if (retained) RedisModule_FreeString(nullptr, retained);
      return REDISMODULE_ERR;
    }
  }

  return REDISMODULE_OK;
}

extern "C" int RedisModule_OnUnload(RedisModuleCtx *ctx) {
  RedisModule_FreeString(ctx, log_key_name);
  if (retained) RedisModule_FreeString(nullptr, retained);

  freeRateLimiter(rate_limiter_handle);

  return REDISMODULE_OK;
}

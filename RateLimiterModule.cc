//
// Created by Rahul  Kushwaha on 2/26/24.
//

#include "/Users/rahulkushwaha/projects/redis/src/redismodule.h"
#include "RateLimiter.h"
#include "glog/logging.h"

#include <atomic>
#include <strings.h>

static RateLimiterHandle rate_limiter_handle;

struct RateLimiterConfig {
  bool enabled;
  bool dryRunEnabled;
  double genRate;
  double burst;
};

RateLimiterConfig current_config{
    .enabled = true,
    .dryRunEnabled = false,
    .genRate = 10,
    .burst = 100,
};

static RedisModuleString* log_key_name;

static const char ratelimiter_block_command_name[] = "ratelimiter.block";
static const char ratelimiter_reset_command_name[] = "ratelimiter.reset";

static int in_log_command = 0;

unsigned long long unfiltered_clientid = 0;

static RedisModuleCommandFilter* filter;
static RedisModuleString* retained;

int CommandFilter_RateLimiterResetCommand(RedisModuleCtx* ctx,
                                          RedisModuleString** argv, int argc) {
  LOG(INFO) << "argc: " << argc;
  if (argc != 3) {
    std::string err{"2 arguments [genRate & burst] not provided"};
    LOG(ERROR) << err;
    RedisModule_ReplyWithError(ctx, err.c_str());
    return REDISMODULE_OK;
  }

  double genRate{0};
  RedisModule_StringToDouble(argv[1], &genRate);

  double burstRate{0};
  RedisModule_StringToDouble(argv[2], &burstRate);

  reset(rate_limiter_handle, genRate, burstRate);

  const char* ok = "OK";
  RedisModule_ReplyWithString(ctx, RedisModule_CreateString(nullptr, ok, 2));

  return REDISMODULE_OK;
}

int CommandFilter_RateLimiterBlockCommand(RedisModuleCtx* ctx,
                                          RedisModuleString** argv, int argc) {
  RedisModule_Log(ctx, "notice", "exceeded quota");
  RedisModule_ReplyWithError(ctx, "TOO_MANY_REQUESTS");

  return REDISMODULE_OK;
}

void CommandFilter_CommandFilter(RedisModuleCommandFilterCtx* filterCtx) {
  unsigned long long id = RedisModule_CommandFilterGetClientId(filterCtx);
  if (id == unfiltered_clientid)
    return;

  if (in_log_command)
    return; /* don't process our own RM_Call() from CommandFilter_LogCommand() */

  if (current_config.enabled && !consume(rate_limiter_handle, 1)) {
    if (current_config.dryRunEnabled) {
      LOG_EVERY_N(INFO, 5) << "command rejected";
      return;
    }

    RedisModule_CommandFilterArgInsert(
        filterCtx, 0,
        RedisModule_CreateString(nullptr, ratelimiter_block_command_name,
                                 sizeof(ratelimiter_block_command_name) - 1));
  }
}

extern "C" {
int RedisModule_OnLoad(RedisModuleCtx* ctx, RedisModuleString** argv,
                       int argc) {
  if (RedisModule_Init(ctx, "commandfilter", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR)
    return REDISMODULE_ERR;

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
                                CommandFilter_RateLimiterBlockCommand,
                                "deny-oom", 1, 1, 1) == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  if (RedisModule_CreateCommand(ctx, ratelimiter_reset_command_name,
                                CommandFilter_RateLimiterResetCommand,
                                "deny-oom", 1, 1, 1) == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  if ((filter = RedisModule_RegisterCommandFilter(
           ctx, CommandFilter_CommandFilter,
           noself ? REDISMODULE_CMDFILTER_NOSELF : 0)) == nullptr)
    return REDISMODULE_ERR;

  if (argc == 3) {
    const char* ptr = RedisModule_StringPtrLen(argv[2], nullptr);
    if (!strcasecmp(ptr, "noload")) {
      /* This is a hint that we return ERR at the last moment of OnLoad. */
      RedisModule_FreeString(ctx, log_key_name);
      if (retained)
        RedisModule_FreeString(nullptr, retained);
      return REDISMODULE_ERR;
    }
  }

  return REDISMODULE_OK;
}

extern "C" int RedisModule_OnUnload(RedisModuleCtx* ctx) {
  RedisModule_FreeString(ctx, log_key_name);
  if (retained)
    RedisModule_FreeString(nullptr, retained);

  freeRateLimiter(rate_limiter_handle);

  return REDISMODULE_OK;
}
}
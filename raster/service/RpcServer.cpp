/*
 * Copyright 2019 Yeolar
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlfcn.h>

#include <gflags/gflags.h>

#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/service/ExecutorFilter.h>
#include <wangle/service/ServerDispatcher.h>
#include <wangle/service/Service.h>

#include <accelerator/cson.h>
#include <accelerator/FileUtil.h>
#include <accelerator/Logging.h>

#include "raster/Context.h"
#include "raster/Scheduler.h"
#include "raster/taskflow/executor.hpp"
#include "ServerSerializeHandler.h"

DEFINE_int32(port, 8080, "server port");
DEFINE_string(conf, "conf/raster.cson", "server conf");

namespace raster {

using SerializePipeline = wangle::Pipeline<folly::IOBufQueue&, Result>;

class RpcService : public wangle::Service<Query, Result> {
 public:
  RpcService() {
    ACCCHECK(initConf());
    ACCCHECK(initDynamic());
    ACCCHECK(initExecutor());
  }

  virtual ~RpcService() {
    if (handle_) {
      dlclose(handle_);
    }
  }

  bool initConf() {
    std::string conf;
    if (!acc::readFile(FLAGS_conf.c_str(), conf)) {
      ACCLOG(ERROR) << "read conf '" << FLAGS_conf << "' failed";
      return false;
    }
    try {
      conf_ = acc::parseCson(conf);
    } catch (std::exception& e) {
      ACCLOG(ERROR) << "parse conf '" << FLAGS_conf << "' failed";
      return false;
    }
    return true;
  }

  bool initDynamic() {
    auto dyn = conf_.getDefault("dllib");
    if (dyn.empty()) {
      ACCLOG(ERROR) << "conf miss dllib: " << acc::toCson(conf_);
      return false;
    }
    handle_ = dlopen(dyn.asString().c_str(), RTLD_LAZY);
    if (handle_ == nullptr) {
      ACCLOG(ERROR) << "dlopen '" << dyn << "' failed: " << dlerror();
      return false;
    }
    return true;
  }

  bool initExecutor() {
    auto threadNum = conf_.getDefault("threadnum");
    if (threadNum.empty()) {
      executor_.reset(new tf::Executor());
    } else {
      int n = threadNum.asInt();
      if (n < 1) {
        ACCLOG(ERROR) << "threadnum: " << n << "<1";
        return false;
      }
      executor_.reset(new tf::Executor(n));
    }
    return true;
  }

  folly::Future<Result> operator()(Query request) override {
    Result response;
    response.set_traceid(request.traceid());
    response.set_code(ResultCode::OK);

    auto sched = conf_.getDefault("scheduler");
    if (sched.empty()) {
      ACCLOG(ERROR) << "conf miss scheduler: " << acc::toCson(conf_);
      response.set_code(ResultCode::E_SCHED__NOTFOUND);
      return response;
    }
    auto schedule = ScheduleManager::getInstance()->get(sched.asString());
    if (!schedule) {
      ACCLOG(ERROR) << "scheduler not registered: " << sched;
      response.set_code(ResultCode::E_SCHED__NOTFOUND);
      return response;
    }
    tf::Taskflow taskflow("raster-taskflow");
    Context context;
    context.request = &request;
    context.response = &response;
    context.conf = &conf_;
    (*schedule)(taskflow, context);
    executor_->run(taskflow).wait();

    return response;
  }

 private:
  acc::dynamic conf_;
  void* handle_;
  std::unique_ptr<tf::Executor> executor_;
};

class RpcPipelineFactory : public wangle::PipelineFactory<SerializePipeline> {
 public:
  SerializePipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransportWrapper> sock) override {
    auto pipeline = SerializePipeline::create();
    pipeline->addBack(wangle::AsyncSocketHandler(sock));
    // ensure we can write from any thread
    pipeline->addBack(wangle::EventBaseHandler());
    pipeline->addBack(wangle::LengthFieldBasedFrameDecoder());
    pipeline->addBack(wangle::LengthFieldPrepender());
    pipeline->addBack(ServerSerializeHandler());
    // We could use a serial dispatcher instead easily
    // pipeline->addBack(SerialServerDispatcher<Query>(&service_));
    // Or a Pipelined Dispatcher
    // pipeline->addBack(PipelinedServerDispatcher<Query>(&service_));
    pipeline->addBack(
        wangle::MultiplexServerDispatcher<Query, Result>(&service_));
    pipeline->finalize();

    return pipeline;
  }

 private:
  wangle::ExecutorFilter<Query, Result> service_{
      std::make_shared<folly::CPUThreadPoolExecutor>(10),
      std::make_shared<RpcService>()};
};

} // namespace raster

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  wangle::ServerBootstrap<raster::SerializePipeline> server;
  server.childPipeline(std::make_shared<raster::RpcPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}

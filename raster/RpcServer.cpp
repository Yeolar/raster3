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

#include "ServerSerializeHandler.h"

DEFINE_int32(port, 8080, "server port");

namespace raster {

using SerializePipeline = wangle::Pipeline<folly::IOBufQueue&, Result>;

class RpcService : public wangle::Service<Query, Result> {
 public:
  folly::Future<Result> operator()(Query request) override {
    printf("Query: %s, %s\n",
           request.traceid().c_str(), request.query().c_str());

    Result response;
    response.set_traceid(request.traceid());
    response.set_code(ResultCode::OK);
    response.set_result(0, "result string");
    return response;
  }
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

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

#pragma once

#include <wangle/channel/Handler.h>

#include "raster/Message.pb.h"

namespace raster {

class ClientSerializeHandler : public wangle::Handler<
  std::unique_ptr<folly::IOBuf>, Result,
  Query, std::unique_ptr<folly::IOBuf>> {
 public:
  void read(Context* ctx, std::unique_ptr<folly::IOBuf> msg) override {
    Result received;
    received.ParseFromArray(msg->data(), msg->length());
    ctx->fireRead(received);
  }

  folly::Future<folly::Unit> write(Context* ctx, Query b) override {
    std::string out;
    b.SerializePartialToString(&out);
    return ctx->fireWrite(folly::IOBuf::copyBuffer(out));
  }
};

} // namespace raster

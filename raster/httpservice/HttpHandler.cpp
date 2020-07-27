/*
 * Copyright 2020 Yeolar
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

#include "HttpHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "HttpStats.h"

namespace raster {

HttpHandler::HttpHandler(crystal::TableFactory* factory,
                         crystal::Graph::Executor* executor,
                         HttpStats* stats)
    : factory_(factory), executor_(executor), stats_(stats) {
}

void HttpHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> /*headers*/)
    noexcept {
  stats_->recordRequest();
}

void HttpHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (body_) {
    body_->prependChain(std::move(body));
  } else {
    body_ = std::move(body);
  }
}

void HttpHandler::onEOM() noexcept {
  crystal::Query query(factory_, executor_, true);
  query += body_->coalesce().str();
  proxygen::ResponseBuilder(downstream_)
    .status(200, "OK")
    .header("Request-Number", folly::to<std::string>(stats_->getRequestCount()))
    .body(folly::IOBuf::copyBuffer(query.runAndToJson(true, true)))
    .sendWithEOM();
}

void HttpHandler::onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void HttpHandler::requestComplete() noexcept {
  delete this;
}

void HttpHandler::onError(proxygen::ProxygenError /*err*/) noexcept {
  delete this;
}

} // namespace raster

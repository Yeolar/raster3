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

#include "raster/rpcservice/Scheduler.h"
#include "ComputeTask.h"
#include "QueryTask.h"
#include "ResultTask.h"

namespace raster {

class SampleScheduler : public Scheduler {
 public:
  void operator()(tf::Taskflow& taskflow,
                  const Context& context) const override {
    auto A = taskflow.emplace(QueryTask(context));
    auto B = taskflow.emplace(ComputeTask(context));
    auto C = taskflow.emplace(ComputeTask(context));
    auto D = taskflow.emplace(ResultTask(context));

    A.precede(B);
    A.precede(C);
    B.precede(D);
    C.precede(D);
  }
};

RASTER_REG_SCHED(SampleScheduler);

} // namespace raster

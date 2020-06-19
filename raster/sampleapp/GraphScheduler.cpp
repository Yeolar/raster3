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

#include "raster/GraphScheduler.h"
#include "ComputeTask.h"
#include "QueryTask.h"
#include "ResultTask.h"

namespace raster {

void schedule(tf::Taskflow& taskflow, const Query& request, Result& response) {
  auto A = taskflow.emplace(QueryTask(request, response));
  auto B = taskflow.emplace(ComputeTask(request, response));
  auto C = taskflow.emplace(ComputeTask(request, response));
  auto D = taskflow.emplace(ResultTask(request, response));

  A.precede(B);
  A.precede(C);
  B.precede(D);
  C.precede(D);
}

} // namespace raster

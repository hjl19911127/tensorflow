/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <string.h>
#include <vector>
#include "tensorflow/contrib/lite/builtin_op_data.h"
#include "tensorflow/contrib/lite/context.h"
#include "tensorflow/contrib/lite/kernels/internal/reference/reference_ops.h"
#include "tensorflow/contrib/lite/kernels/internal/tensor.h"
#include "tensorflow/contrib/lite/kernels/kernel_util.h"
#include "tensorflow/contrib/lite/kernels/op_macros.h"

namespace tflite {
namespace ops {
namespace builtin {
namespace fake_quant {

// This file has reference implementation of FakeQuant.
enum KernelType {
  kReference,
};

struct OpContext {
  OpContext(TfLiteContext* context, TfLiteNode* node) {
    input = GetInput(context, node, 0);
    output = GetOutput(context, node, 0);
  }
  const TfLiteTensor* input;
  TfLiteTensor* output;
};

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  OpContext op_context(context, node);
  TfLiteIntArray* output_dims = TfLiteIntArrayCopy(op_context.input->dims);
  op_context.output->type = op_context.input->type;
  return context->ResizeTensor(context, op_context.output, output_dims);
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  OpContext op_context(context, node);

  const auto* params =
      reinterpret_cast<TfLiteFakeQuantParams*>(node->builtin_data);

  reference_ops::FakeQuant(GetTensorData<float>(op_context.input),
                           GetTensorDims(op_context.input), params->min,
                           params->max, params->num_bits,
                           GetTensorData<float>(op_context.output),
                           GetTensorDims(op_context.output));

  return kTfLiteOk;
}

}  // namespace fake_quant

TfLiteRegistration* Register_FAKE_QUANT_REF() {
  static TfLiteRegistration r = {nullptr, nullptr, fake_quant::Prepare,
                                 fake_quant::Eval<fake_quant::kReference>};
  return &r;
}

TfLiteRegistration* Register_FAKE_QUANT() { return Register_FAKE_QUANT_REF(); }

}  // namespace builtin
}  // namespace ops
}  // namespace tflite

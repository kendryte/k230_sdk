import torch
import numpy as np
import random
import onnxruntime as ort
def set_random_seed(seed=0):
    ort.set_seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    torch.backends.cudnn.deterministic = True
    random.seed(seed)
    np.random.seed(seed)

def runonnx(model_path, **kwargs):
    ort_session = ort.InferenceSession(model_path)
    outputs = ort_session.run(
        None,
        kwargs
    )
    return outputs
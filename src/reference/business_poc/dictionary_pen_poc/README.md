## 简介

词典笔项目接口，具体接口见文档：https://cf.b-bug.org/pages/viewpage.action?pageId=108174084

## 运行
根据不同操作可修改main.cpp  
根据sdk路径修改CMakeList.txt中的DEMO_ROOT

```
bash build.sh
```

### 拼接

```
#include "stitch_api.hpp"
stitch_api("./testdata/Fri_Mar_24_00_47_10_2023_","./result_stitch.jpg");
```

### OCR

```
#include "ocr_api.h"
ocr_api ocr_api;
ocr_api.ocr("./result_stitch.jpg","./result_ocr.txt");
```

### TTS

```
#include "tts_api.h"
string audio_name = "audio.wav";
tts_api tts_api;
tts_api.tts("good morning !", audio_name);
```

### NMT

```
#include "nmt_api.h"
nmt_api nmt_api;
//英翻中为false，中翻英为true
nmt_api.nmt("good morning !","result_nmt.txt",false);
```

### pipeline

```
#include "stitch_api.hpp"
#include "ocr_api.h"
#include "tts_api.h"
#include "nmt_api.h"

cidianbi_pipeline("./testdata/Fri_Mar_24_00_47_10_2023_","audio.wav");
```

git lfs fetch
git lfs pull
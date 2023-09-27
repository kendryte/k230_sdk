/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "my_app.h"
#include "media.h"

int main() {
    MyApp *app = new MyApp(16);
    TrigerMode triger_mode = app->GetCurrentMode();
    // TrigerMode triger_mode = DOORBELL_MODE;
    printf("triger mode = %d...\n", triger_mode);
    k_s32 ret = kd_mapi_sys_init();
    std::cout << "playback mode sys init ret = " << ret << std::endl;
    app->Init();

    if (triger_mode == PIR_MODE) {
        TrigerMode mode_switch = app->EnterPirMode();
        printf("mode_switch = %d...\n", mode_switch);

        if (mode_switch == DOORBELL_MODE) {
            mode_switch = app->EnterDoorBellMode();
        }

        if (mode_switch == PLAYBACK_MODE) {
            app->EnterPlaybackMode();
        }
    } else if (triger_mode == DOORBELL_MODE) {
        TrigerMode mode_switch = app->EnterDoorBellMode();
        if (mode_switch == PLAYBACK_MODE) {
            app->EnterPlaybackMode();
        }
    }

    app->DeInit();
    delete app;

    kd_mapi_sys_deinit();
    printf("MyApp quit...\n");
    return 0;
}

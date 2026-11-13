#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// キーボードの状態を更新(毎フレーム呼び出す)
void KSMPlatformLinux_UpdateKeyboardState();

// プラットフォーム特有のキー状態を取得(X11のkeysymを指定)
bool KSMPlatformLinux_IsKeyPressed(unsigned long keysym);

#ifdef __cplusplus
}
#endif

#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// IMEを無効化し、ASCII入力ソースに切り替える
void KSMPlatformMacOS_DisableIME();

// プラットフォーム特有のキー状態を取得(仮想キーコードを指定)
bool KSMPlatformMacOS_IsKeyPressed(int keyCode);

// 英数・かなキーのイベントタップを開始(IME切り替えを防ぐ)
void KSMPlatformMacOS_StartBlockingIMEKeys();

// 英数・かなキーのイベントタップを停止
void KSMPlatformMacOS_StopBlockingIMEKeys();

// テキスト編集中かどうかを設定(テキスト編集中はIMEキーをブロックしない)
void KSMPlatformMacOS_SetIsEditingText(bool isEditing);

// ディレクトリを再帰的にコピー(成功時true, 失敗時false)
bool KSMPlatformMacOS_CopyDirectory(const char* srcPath, const char* dstPath);

// メインウィンドウをアクティブにする
void KSMPlatformMacOS_ActivateWindow();

#ifdef __cplusplus
}
#endif

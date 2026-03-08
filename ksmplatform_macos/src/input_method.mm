#include "ksmplatform_macos/input_method.h"
#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include <CoreGraphics/CoreGraphics.h>

namespace
{
	// 指定されたIDの入力ソースを検索
	TISInputSourceRef FindInputSourceByID(CFStringRef sourceID)
	{
		if (sourceID == nullptr)
		{
			return nullptr;
		}

		CFStringRef keys[] = { kTISPropertyInputSourceID };
		CFStringRef values[] = { sourceID };
		CFDictionaryRef dict = CFDictionaryCreate(
			kCFAllocatorDefault,
			reinterpret_cast<const void**>(keys),
			reinterpret_cast<const void**>(values),
			1,
			&kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks
		);

		CFArrayRef sources = TISCreateInputSourceList(dict, false);
		CFRelease(dict);

		if (sources == nullptr || CFArrayGetCount(sources) == 0)
		{
			if (sources)
			{
				CFRelease(sources);
			}
			return nullptr;
		}

		TISInputSourceRef result = static_cast<TISInputSourceRef>(
			const_cast<void*>(CFArrayGetValueAtIndex(sources, 0))
		);

		// 結果を保持してから配列を解放
		if (result)
		{
			CFRetain(result);
		}
		CFRelease(sources);

		return result;
	}

	// 現在の入力ソースのASCII/Romanモードを探す
	TISInputSourceRef FindASCIIModeOfCurrentSource(CFStringRef currentSourceID)
	{
		if (currentSourceID == nullptr)
		{
			return nullptr;
		}

		@autoreleasepool
		{
			NSString* sourceIDStr = (__bridge NSString*)currentSourceID;

			// ATOK ("com.justsystems.inputmethod.atok*.Japanese"の場合は".Japanese"から".Roman"へ変更)
			if ([sourceIDStr containsString:@"atok"] && [sourceIDStr hasSuffix:@".Japanese"])
			{
				NSString* romanID = [sourceIDStr stringByReplacingOccurrencesOfString:@".Japanese"
					withString:@".Roman"];
				TISInputSourceRef romanSource = FindInputSourceByID((__bridge CFStringRef)romanID);
				if (romanSource)
				{
					return romanSource;
				}
			}

			return nullptr;
		}
	}
}

void KSMPlatformMacOS_DisableIME()
{
	// 現在の入力ソースを取得
	TISInputSourceRef currentSource = TISCopyCurrentKeyboardInputSource();
	if (currentSource == nullptr)
	{
		return;
	}

	// 現在の入力ソースが既にASCII対応か確認
	CFBooleanRef isASCIICapable = static_cast<CFBooleanRef>(
		TISGetInputSourceProperty(currentSource, kTISPropertyInputSourceIsASCIICapable)
	);

	if (isASCIICapable && CFBooleanGetValue(isASCIICapable))
	{
		// 既にASCII対応の入力ソースなので何もしない
		CFRelease(currentSource);
		return;
	}

	// 現在の入力ソースIDを取得
	CFStringRef currentSourceID = static_cast<CFStringRef>(
		TISGetInputSourceProperty(currentSource, kTISPropertyInputSourceID)
	);

	// 同じ入力メソッド内のASCII/Romanモードを探す
	TISInputSourceRef asciiModeSource = FindASCIIModeOfCurrentSource(currentSourceID);

	if (asciiModeSource)
	{
		// 見つかった場合はそのモードに切り替え
		TISSelectInputSource(asciiModeSource);
		CFRelease(asciiModeSource);
		CFRelease(currentSource);
		return;
	}

	// ASCII/Romanモードが見つからない場合は最初のASCII入力ソースを選択(通常は英語キーボード)
	CFArrayRef asciiSources = TISCreateASCIICapableInputSourceList();
	if (asciiSources != nullptr)
	{
		if (CFArrayGetCount(asciiSources) > 0)
		{
			TISInputSourceRef asciiSource = static_cast<TISInputSourceRef>(
				const_cast<void*>(CFArrayGetValueAtIndex(asciiSources, 0))
			);
			TISSelectInputSource(asciiSource);
		}
		CFRelease(asciiSources);
	}

	CFRelease(currentSource);
}

bool KSMPlatformMacOS_IsKeyPressed(int keyCode)
{
	// CGKeyCodeは0〜127の範囲なので、範囲チェック
	if (keyCode < 0 || keyCode > 127)
	{
		return false;
	}

	return CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, static_cast<CGKeyCode>(keyCode));
}

namespace
{
	id s_eventMonitor = nil;
	bool s_isEditingText = false;
}

void KSMPlatformMacOS_StartBlockingIMEKeys()
{
	// 既に開始している場合は何もしない
	if (s_eventMonitor != nil)
	{
		return;
	}

	// NSEventのローカルモニターを追加
	// flagsChangedも監視してmodifierキーと組み合わせた場合にも対応
	s_eventMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:(NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged)
		handler:^NSEvent*(NSEvent* event)
		{
			// キーダウン・キーアップイベントのみ処理
			if (event.type != NSEventTypeKeyDown && event.type != NSEventTypeKeyUp)
			{
				return event;
			}

			unsigned short keyCode = [event keyCode];

			// 英数(0x66)またはかな(0x68)キーの場合
			if (keyCode == kVK_JIS_Eisu || keyCode == kVK_JIS_Kana)
			{
				// テキスト編集中の場合はイベントを通す(IME切り替えを許可)
				if (s_isEditingText)
				{
					return event;
				}

				// テキスト編集中でない場合はイベントを破棄(IME切り替えを防ぐ)
				return nil;
			}

			return event;
		}];
}

void KSMPlatformMacOS_StopBlockingIMEKeys()
{
	// イベントモニターを解除
	if (s_eventMonitor != nil)
	{
		[NSEvent removeMonitor:s_eventMonitor];
		s_eventMonitor = nil;
	}
}

void KSMPlatformMacOS_SetIsEditingText(bool isEditing)
{
	s_isEditingText = isEditing;
}

void KSMPlatformMacOS_ActivateWindow()
{
	[NSApp activateIgnoringOtherApps:YES];
	[[NSApp mainWindow] makeKeyAndOrderFront:nil];
}

bool KSMPlatformMacOS_CopyDirectory(const char* srcPath, const char* dstPath)
{
	if (srcPath == nullptr || dstPath == nullptr)
	{
		return false;
	}

	@autoreleasepool
	{
		NSString* source = [NSString stringWithUTF8String:srcPath];
		NSString* destination = [NSString stringWithUTF8String:dstPath];

		NSFileManager* fileManager = [NSFileManager defaultManager];
		NSError* error = nil;

		// copyItemAtPathを使って再帰的にコピー
		BOOL success = [fileManager copyItemAtPath:source toPath:destination error:&error];

		if (!success && error != nil)
		{
			NSLog(@"Failed to copy directory: %@", [error localizedDescription]);
			return false;
		}

		return success;
	}
}

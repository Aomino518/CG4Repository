#pragma once
#include <string.h>
#include <fstream>


class Logger {
public:
	enum class LogLevel {
		Debug,
		Info,
		Warning,
		Error
	};

	static void Init(); // 初期化
	static void Shutdown(); // 終了処理
	static void Write(const std::string& msg); // 書き込み

	static void Write(LogLevel level, const std::string& msg);
	static void SetLevel(LogLevel level);

private:
	static std::ofstream stream_; // 出力先のファイルストリーム
	static LogLevel currentLevel_;

	static constexpr int kMaxLogFiles = 10;

	static void RemoveOldLogs();
};

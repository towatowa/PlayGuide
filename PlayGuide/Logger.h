#pragma once
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>
#include <ostream>
#include <queue>
#include <sstream>
#include <filesystem>
#include "Windows.h"
#include "utils.h"

#ifdef _DEBUG
#define LOG_DEBUG LogStream(LogLevel::Debug)
#define LOG_DEBUG_MSG(msg) Logger::Instance().Log(LogLevel::Debug, msg)
#else
#define LOG_DEBUG LogStream::Null()
#define LOG_DEBUG_MSG(msg) ((void)0)
#endif

#ifdef _LOG_INFO
#define LOG_INFO  LogStream(LogLevel::Info)
#define LOG_INFO_MSG(msg) Logger::Instance().Log(LogLevel::Info, msg)
#else
#define LOG_INFO LogStream::Null()
#define LOG_INFO_MSG(msg) ((void)0)
#endif
#ifdef _LOG_WARN
#define LOG_WARN  LogStream(LogLevel::Warn)
#define LOG_WARN_MSG(msg) Logger::Instance().Log(LogLevel::Warn, msg)
#else
#define LOG_WARN LogStream::Null()
#define LOG_WARN_MSG(msg) ((void)0)
#endif

#define LOG_ERROR LogStream(LogLevel::Error)
#define LOG_ERROR_MSG(msg) Logger::Instance().Log(LogLevel::Error, msg)

typedef std::chrono::time_point<std::chrono::system_clock> TimePoint;
enum class LogLevel
{
	Debug,
	Info,
	Warn,
	Error
};
struct LogMessage {
	LogLevel level;
	std::string text;
	std::chrono::system_clock::time_point time;
	DWORD thread_id;
};

class LogSink
{
public:
	virtual ~LogSink() = default;
	virtual void Write(const LogMessage&) noexcept = 0;
	static std::string GetTimeString(const TimePoint& time);
	static std::string GetLevelToString(LogLevel level);
};

class StdoutSink : public LogSink
{
public:
	void Write(const LogMessage&) noexcept override;
};

class DebugOutputSink : public LogSink
{
public:
	void Write(const LogMessage&) noexcept override;
};

class FileSink : public LogSink
{
public:
	FileSink(const std::wstring& filePath);
	~FileSink();
	void Write(const LogMessage&) noexcept override;
private:
	std::ofstream  m_logFile;
	std::mutex m_mutex;
};

class Logger {
public:

	static Logger& Instance();
	void Log(LogLevel level, const std::string&);
	void AddSink(std::unique_ptr<LogSink> sink);

	void SetMinLevel(LogLevel level);

	LogLevel GetMinLevel() const;

	void Push(const LogMessage&&);
	void Start() noexcept;
	void Stop();

private:
	void Worker();

private:
	Logger() = default;
	~Logger();

	std::vector<std::unique_ptr<LogSink>> m_sinks;
	std::mutex m_mutex;
	LogLevel m_minLevel{ LogLevel::Debug };

	std::queue<LogMessage>m_queue;
	std::condition_variable m_cv;
	std::thread m_thread;

	std::atomic_bool m_running{ false };
};

struct NullLogStream {
	template <typename T>
	NullLogStream& operator<<(T&&) noexcept {
		return *this;
	}
};

class LogStream {
public:
	LogStream(LogLevel level);

	~LogStream();

	template<typename T>
	LogStream& operator<<(const T& value);
	LogStream& operator<<(const std::wstring& value) {
		m_stream << utils::WstringToString(value);
		return *this;
	}
	LogStream& operator<<(const wchar_t* value) {
		if (value)
			m_stream << utils::WstringToString(std::wstring(value));
		else
			m_stream << "(null)";
		return *this;
	}
	static NullLogStream Null();

private:
	LogLevel m_level;
	std::ostringstream m_stream;
	TimePoint m_time;
};

template <typename T>
LogStream& LogStream::operator<<(const T& value)
{
	m_stream << value;
	return *this;
}

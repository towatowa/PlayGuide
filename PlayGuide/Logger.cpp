#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>


std::string LogSink::GetTimeString(const TimePoint&time)
{
	auto t = std::chrono::system_clock::to_time_t(time);

	std::tm tm{};
	localtime_s(&tm, &t);

	std::stringstream ss;
	ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

std::string LogSink::GetLevelToString(LogLevel level)
{
	switch (level) 
	{
	case LogLevel::Debug: return "DEBUG";
	case LogLevel::Info:  return "INFO";
	case LogLevel::Warn:  return "WARN";
	case LogLevel::Error: return "ERROR";
	default:              return "UNKNOWN";
	}
}

void StdoutSink::Write(const LogMessage& msg) noexcept
{
	std::string out = "[" + GetTimeString(msg.time) + "][" + GetLevelToString(msg.level) + "] " + msg.text;
	std::wcout << utils::StringToWstring(out);
}

void DebugOutputSink::Write(const LogMessage& msg) noexcept
{
	std::string out = "[" + GetTimeString(msg.time) + "][" + GetLevelToString(msg.level) + "] " + msg.text;
	OutputDebugString(utils::StringToWstring(out).c_str());
}

FileSink::FileSink(const std::wstring& filePath)
{
	try {
		std::filesystem::path p(filePath);
		std::filesystem::create_directories(p.parent_path());

		m_logFile.imbue(std::locale(".UTF8"));
		m_logFile.open(p, std::ios::app);

		if (!m_logFile.is_open()) {
			OutputDebugStringW(L"[Log] Failed to open log file\n");
		}
	}
	catch (const std::exception& e) {
		OutputDebugStringA(e.what());
	}
}

FileSink::~FileSink()
{
}

void FileSink::Write(const LogMessage& msg) noexcept
{
	try {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_logFile.is_open())
			return;
		m_logFile << "[" + GetTimeString(msg.time) + "] "
			+ "[" + GetLevelToString(msg.level) + "] "
			+ msg.text;
		m_logFile.flush();//flush write to file or use std::end 
	   
	}
	catch (...) {
		// 吃掉异常，日志不能影响主流程
	}
}

Logger::~Logger()
{
	Stop();
}

Logger& Logger::Instance()
{
	static Logger inst;
	return inst;
}

void Logger::Log(LogLevel level, const std::string&str)
{
	Push({ level, str, std::chrono::system_clock::now(), GetCurrentThreadId()});
}

void Logger::AddSink(std::unique_ptr<LogSink> sink)
{
	m_sinks.emplace_back(std::move(sink));
}

void Logger::SetMinLevel(LogLevel level)
{
	m_minLevel = level;
}

LogLevel Logger::GetMinLevel() const
{
	return m_minLevel;
}

void Logger::Push(const LogMessage&&msg)
{
	{
		std::lock_guard<std::mutex>lock(m_mutex);
		m_queue.push(std::move(msg));
	}
	m_cv.notify_one();
}

void Logger::Start() noexcept
{
	m_running = true;
	
	m_thread = std::thread(&Logger::Worker, this);
}

void Logger::Stop()
{
	if (!m_running) return;
	m_running = false;
	m_cv.notify_all();
	if (m_thread.joinable())
		m_thread.join();
}

void Logger::Worker()
{
	while (m_running || !m_queue.empty()) 
	{
		std::unique_lock<std::mutex>lock(m_mutex);
		m_cv.wait(lock, [&] {
			return !m_running || !m_queue.empty(); 
			});
		while (!m_queue.empty()) {
			auto msg = std::move(m_queue.front());
			m_queue.pop();
			lock.unlock();

			for (auto& s : m_sinks)
				s->Write(msg);
			lock.lock();
		}

	}
}

LogStream::LogStream(LogLevel level):m_level(level), m_time(std::chrono::system_clock::now())
{
}

LogStream::~LogStream()
{
	if (m_level >= Logger::Instance().GetMinLevel()) {
		Logger::Instance().Push({m_level, m_stream.str(), m_time, GetCurrentThreadId()});
	}
}

NullLogStream LogStream::Null()
{
	return NullLogStream();
}

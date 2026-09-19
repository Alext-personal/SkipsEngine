#pragma once
#include <iostream>
#include <chrono>
#include <string>
#include <queue>
#include <source_location>
namespace Gaze {
	class Log {
	public:
		static void Log_Time(std::ostream& out) {
			const auto utcTime{ std::chrono::system_clock::now() };
			const auto localTime{ std::chrono::floor<std::chrono::seconds>(std::chrono::current_zone()->to_local(utcTime)) };
			out << "[" << localTime << "] : " << " ";
		}
		template <typename... Args>
		static void LogError(const std::source_location& location, const Args&... args) {
			m_Log("ERROR", location, args...);
		}
		template <typename... Args>
		static void LogWarning(const std::source_location& location, const Args&... args) {
			m_Log("WARNING", location, args...);
		}
		template <typename... Args>
		static void LogInfo(const std::source_location& location,const Args&... args) {
			std::cout << Green << "[INFO] " << White;
			Log_Time(std::cout);
			std::string message = ParseArguments(location,args...);
			std::cout << message << "\n";
		}
		static void LogFallback(const std::string& message,const std::source_location& location) {
			std::cerr << Purple << "[FILE] : " << White << location.file_name() << " ";
			std::cerr << Purple << "[LINE] : " << White << location.line() << "\n";
			std::cout << Purple << "[LOGGER ERROR] " << White;
			Log_Time(std::cout);
			std::cout << message << "\n";
			
		}
		template <typename... Args>
		static void ASSERT(const std::string& type, bool condition, const std::source_location& location, const Args&... args) {
			if (!condition) {
				m_Log("CRITICAL", location, args...);
				if (type == "ENGINE")
					std::abort();
				if (type == "CLIENT")
					throw std::runtime_error(ParseArguments(location,args...));
			}
		}
	private:
		inline static const char* White{ "\033[0m" };
		inline static const char* Red{ "\033[31m" };
		inline static const char* Yellow{ "\033[33m" };
		inline static const char* Green{ "\033[32m" };
		inline static const char* DarkRed{ "\033[38;5;88m" };
		inline static const char* Purple{ "\033[95m" };
		template <typename... Args>
		static void m_Log(const std::string& type, const std::source_location& location, const Args&... args) {
			const char* colour = Green;
			if (type == "ERROR")
				colour = Red;
			if (type == "WARNING")
				colour = Yellow;
			if (type == "CRITICAL")
				colour = DarkRed;
			std::cerr << colour << "[FILE] : " << White << location.file_name() << " ";
			std::cerr << colour << "[LINE] : " << White << location.line() << "\n";
			std::cerr << colour << "[" << type << "] : " << White;
			Log_Time(std::cerr);
			std::string message = ParseArguments(location,args...);
			std::cerr << message << "\n";
		}
		template <typename... Args>
		static std::string ParseArguments(const std::source_location& location, const Args&... args) {
			std::string returnedString;
			bool isFirst = true;
			auto argsTuble = std::forward_as_tuple(args...);
			size_t searchPosition = 0;
			auto process = [&](auto&& arg)
				{
					if (isFirst)
					{
						returnedString = ToString(arg);
						isFirst = false;
						return;
					}

					size_t pos = returnedString.find("${}", searchPosition);

					if (pos == std::string::npos)
					{
						LogFallback("INVALID LOGGER FORMAT", location);
					}

					std::string value = ToString(arg);

					returnedString.replace(pos, 3, value);

					searchPosition = pos + value.size();
				};
			std::apply([&](auto&&... args)
				{
					(process(args), ...);

				}, argsTuble);
			return returnedString;
		}
		template <typename T>
		static std::string ToString(const T& value) {
			std::stringstream temp;
			temp << value;
			return temp.str();
		}
	};
#define LOG_ERROR(...) \
	Log::LogError(std::source_location::current(), __VA_ARGS__)
#define LOG_WARNING(...) \
	Log::LogWarning(std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...)\
	Log::LogInfo(std::source_location::current(),__VA_ARGS__)
#define ENGINE_ASSERT(condition,...)\
	Log::ASSERT("ENGINE",condition,std::source_location::current(),__VA_ARGS__)
#define CLIENT_ASSERT(condition,...)\
	Log::ASSERT("CLIENT",condition,std::source_location::current(),__VA_ARGS__)
}
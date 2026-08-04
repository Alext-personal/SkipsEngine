#pragma once
#include <iostream>
#include <chrono>
class Log {
public:
	inline static const char* White{ "\033[0m" };
	inline static const char* Red{ "\033[31m" };
	inline static const char* Yellow{ "\033[33m" };
	inline static const char* Green{ "\033[32m" };
	static void Log_Time(std::ostream& out) {
		const auto utcTime{ std::chrono::system_clock::now() };
		const auto localTime{ std::chrono::floor<std::chrono::seconds>(std::chrono::current_zone()->to_local(utcTime)) };
		out << "[" << localTime << "] : " << " ";
	}
	template <typename... Args>
	static void ERROR(const Args&... args) {
		std::cerr << Red << "[ERROR] " << White;
		Log_Time(std::cerr);
		(std::cerr << ... << args);
		std::cerr << "\n";
	}
	template <typename... Args>
	static void WARNING(const Args&... args) {
		std::cerr << Yellow << "[WARNING] " << White;
		Log_Time(std::cerr);
		(std::cerr << ... << args);
		std::cerr << "\n";
	}
	template <typename... Args>
	static void INFO(const Args&... args) {
		std::cout << Green << "[INFO] " << White;
		Log_Time(std::cout);
		(std::cout << ... << args);
		std::cout << "\n";
	}
};
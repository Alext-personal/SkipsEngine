#pragma once
#include <iostream>
#include <chrono>
void Log_Time(std::ostream &out) {
	const auto utcTime{ std::chrono::system_clock::now() };
	const auto localTime{ std::chrono::floor<std::chrono::seconds>(std::chrono::current_zone()->to_local(utcTime)) };
	out << "[" <<localTime<< "] : " << " ";
}
const char* White{ "\033[0m" };
const char* Red{ "\033[31m" };
const char* Yellow{ "\033[33m" };
const char* Green{ "\033[32m" };
#define LOG_ERROR(errormsg)\
		std::cerr<<Red<<"[ERROR] "<<White;\
		Log_Time(std::cerr);\
		std::cerr<<errormsg<<"\n";

#define LOG_WARNING(warningmsg)\
		std::cout<<Yellow<<"[WARNING] "<<White;\
		Log_Time(std::cout);\
		std::cout<<warningmsg<<"\n";

#define LOG_INFO(infomsg)\
		std::cout<<Green<<"[INFO] "<<White;\
		Log_Time(std::cout);\
		std::cout<<infomsg<<"\n";

		
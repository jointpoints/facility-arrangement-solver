/**
 * @file Logger.cpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#include "Logger.hpp"
#include <filesystem>
#include <format>
#include <chrono>
#include <vector>





std::string const inline timestamp(void)
{
	auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm local_time = *std::localtime(&time);
	return std::format
	(
		"[{}-{}-{} {}:{}:{}]",
		std::to_string(local_time.tm_year + 1900),
		(local_time.tm_mon < 10 ? "0" : "") + std::to_string(local_time.tm_mon + 1),
		(local_time.tm_mday < 10 ? "0" : "") + std::to_string(local_time.tm_mday),
		(local_time.tm_hour < 10 ? "0" : "") + std::to_string(local_time.tm_hour),
		(local_time.tm_min < 10 ? "0" : "") + std::to_string(local_time.tm_min),
		(local_time.tm_sec < 10 ? "0" : "") + std::to_string(local_time.tm_sec)
	);
}





// Methods of Logger





Logger::Logger(std::string const path, int& status_code) noexcept
{
	// Create a path object out of `path` and test it
	std::filesystem::path output_file_path(path);
	if (!output_file_path.has_filename()) {status_code = FASOLVER_LOGGER_STATUS_BAD_FILENAME; return;}
	if (!output_file_path.is_absolute())
		output_file_path = std::filesystem::current_path() / output_file_path;
	try
	{
		if (output_file_path.has_relative_path() && !std::filesystem::exists(output_file_path.parent_path()))
			std::filesystem::create_directory(output_file_path.parent_path());
	}
	catch (...) {status_code = FASOLVER_LOGGER_STATUS_BAD_PARENT; return;}

	// Open the output stream
	this->output_stream.open(path, std::ios::out);
	if (!output_stream.is_open()) {status_code = FASOLVER_LOGGER_STATUS_BAD_FILE; return;}

	// Assign the new stream to the universal interface
	this->core.reset(new LoggerCore(this->output_stream));

	// Create a callback
	this->info_callback.reset(new LoggerOStream(this, &Logger::info));

	status_code = FASOLVER_LOGGER_STATUS_OK;

	return;
}





Logger::Logger(std::basic_ostream<char8_t>& output_stream, int& status_code) noexcept
{
	// Assign the given stream to the universal interface
	this->core.reset(new LoggerCore(output_stream));

	// Create a callback
	this->info_callback.reset(new LoggerOStream(this, &Logger::info));

	status_code = FASOLVER_LOGGER_STATUS_OK;
	return;
}





Logger::~Logger(void)
{
	if (this->output_stream.is_open())
		this->output_stream.close();
	return;
}





void Logger::info(std::string const message) const
{
	this->core->info(message);
	return;
}





std::ostream& Logger::getInfoCallback(void) const
{
	return *this->info_callback;
}





// Methods of Logger::LoggerStreamBuf





Logger::LoggerStreamBuf::int_type Logger::LoggerStreamBuf::overflow(int_type c)
{
	if (!traits_type::eq_int_type(c, traits_type::eof()))
	{
		*this->pptr() = traits_type::to_char_type(c);
		this->pbump(1);
	}
	return this->sync() ? traits_type::not_eof(c) : traits_type::eof();
}





int Logger::LoggerStreamBuf::sync(void)
{
	if (this->pbase() != this->pptr())
	{
		// Split message from the buffer into lines
		std::vector<std::string> lines;
		char *begin = this->pbase();
		char *end = this->pbase();
		while (end != this->pptr())
			if (*end == '\n')
			{
				lines.emplace_back(begin, end);
				begin = end + 1;
				end = begin;
			}
			else
				++end;
		// Send all lines to the callback
		for (auto const& line : lines)
			(this->logger->*callback_function)(line);
		// Reset buffer
		this->setp(this->pbase(), this->epptr());
	}
	return 0;
}





Logger::LoggerStreamBuf::LoggerStreamBuf(Logger* const logger, void (Logger::*callback_function)(std::string const) const)
	: logger(logger)
	, callback_function(callback_function)
{
	this->setp(this->buffer, this->buffer + sizeof(this->buffer) - 1);
	return;
}





// Methods of Logger::LoggerOStream





//Logger::LoggerOStream::LoggerOStream(Logger* const logger)
Logger::LoggerOStream::LoggerOStream(Logger* const logger, void (Logger::*callback_function)(std::string const) const)
	: basic_ostream(static_cast<std::streambuf*>(this))
	, LoggerStreamBuf(logger, callback_function)
{
	this->flags(std::ios_base::unitbuf);
	return;
}





// Methods of Logger::LoggerCore





Logger::LoggerCore::LoggerCore(std::basic_ostream<char8_t>& output_stream)
	: output_stream(output_stream) {}





void Logger::LoggerCore::info(std::string const message) const
{
	this->output_stream << (timestamp() + " INFO    | " + message + "\n").data();
	return;
}

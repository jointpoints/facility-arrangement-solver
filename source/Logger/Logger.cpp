/**
 * @file Logger.cpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#include "Logger.hpp"
#include <filesystem>
#include <format>
#include <chrono>





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
	this->callback.reset(new LoggerOStream(this));

	status_code = FASOLVER_LOGGER_STATUS_OK;

	return;
}





Logger::Logger(std::basic_ostream<char8_t>& output_stream, int& status_code) noexcept
{
	// Assign the given stream to the universal interface
	this->core.reset(new LoggerCore(output_stream));

	// Create a callback
	this->callback.reset(new LoggerOStream(this));

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
	return *this->callback;
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
		logger->info(std::string(this->pbase(), this->pptr()));
		this->setp(this->pbase(), this->epptr());
	}
	return 0;
}





Logger::LoggerStreamBuf::LoggerStreamBuf(Logger* const logger)
	: logger(logger)
{
	this->setp(this->buffer, this->buffer + sizeof(this->buffer) - 1);
	return;
}





// Methods of Logger::LoggerOStream





Logger::LoggerOStream::LoggerOStream(Logger* const logger)
	: basic_ostream(static_cast<std::basic_streambuf<char8_t>*>(this))
	, LoggerStreamBuf(logger)
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

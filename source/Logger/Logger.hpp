/**
 * @file Logger.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_LOGGER_HPP__
#define __FASOLVER_LOGGER_HPP__





#include <fstream>
#include <memory>
#include <string>





#define FASOLVER_LOGGER_TO_CONSOLE std::cout
#define FASOLVER_LOGGER_NO_LOG     ""

#define FASOLVER_LOGGER_STATUS_OK           0b000
#define FASOLVER_LOGGER_STATUS_BAD_PARENT   0b001
#define FASOLVER_LOGGER_STATUS_BAD_FILENAME 0b010
#define FASOLVER_LOGGER_STATUS_BAD_FILE     0b100





/**
 * @class Logger
 * @brief Logs actions of the program
 *
 * Objects of this class output the text data related to the execution of the program.
 */
class Logger final
{
	/// File output stream of log (only used when logging is performed into the file)
	std::basic_ofstream<char8_t> output_stream;

	class LoggerCore;
	class LoggerStreamBuf;
	class LoggerOStream;

	/// A unique pointer to the core logger interface
	std::unique_ptr<LoggerCore> core;

	/// A callback output stream to redirect log data of, e.g., some library into this logger
	std::unique_ptr<LoggerOStream> callback;



public:
	/// @name Constructors & destructors
	/// @{

	Logger(void) = delete;
	Logger(Logger&) = delete;
	Logger(Logger&&) = delete;

	/**
	 * @brief Construct a logger for the given file
	 *
	 * Creates a new logger connected to the specified file.
	 */
	explicit
	Logger(std::string const path, int& status_code) noexcept;

	/**
	 * @brief Construct a logger to the given stream
	 *
	 * Creates a new logger connected to the specified output stream.
	 */
	explicit
	Logger(std::basic_ostream<char8_t>& output_stream, int& status_code) noexcept;

	/**
	 * @brief Destroy the logger
	 *
	 * Properly destroys the logger object.
	 */
	~Logger(void);

	/// @}



	/// @name I/O
	/// @{

	void info(std::string const message) const;

	std::ostream const& getInfoCallback(void) const;

	/// @}
};





class Logger::LoggerStreamBuf
	: public std::basic_streambuf<char8_t>
{
	/// A pointer to the enclosing object
	Logger* const logger;
	/// Buffer
	char_type buffer[1024];

	/// Overload of the `overflow` method
	int_type overflow(int_type c) override;
	/// Overload of the `sync` method
	int sync(void) override;



public:
	/// @name Constructors & destructors
	/// @{

	/**
	 * @brief Constructor
	 *
	 * Constructs a new log callback stream buffer.
	 */
	LoggerStreamBuf(Logger* const logger);

	/// @}
};





class Logger::LoggerOStream final
	: public std::basic_ostream<char8_t>,
	  private virtual Logger::LoggerStreamBuf
{
public:
	/// @name Constructors & destructors
	/// @{

	/**
	 * @brief Constructor
	 *
	 * Constructs a new log callback stream.
	 */
	LoggerOStream(Logger* const logger);

	/// @}
};





class Logger::LoggerCore final
{
	/// The output stream
	std::basic_ostream<char8_t>& output_stream;



public:
	/// @name Constructors and destructors
	/// @{

	LoggerCore(void) = delete;

	/**
	 * @brief Construct a logger core
	 *
	 * Creates a core logger interface for the given output stream.
	 */
	LoggerCore(std::basic_ostream<char8_t>& output_stream);
	
	/// @}



	/// @name
	/// @{

	void info(std::string const message) const;

	/// @}
};





#endif // __FASOLVER_LOGGER_HPP__

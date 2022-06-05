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

	/// A callback output stream to redirect info log data of, e.g., some library into this logger
	std::unique_ptr<std::ostream> info_callback;
	/// A callback output stream to redirect warning log data of, e.g., some library into this logger
	std::unique_ptr<std::ostream> warning_callback;
	/// A callback output stream to redirect error log data of, e.g., some library into this logger
	std::unique_ptr<std::ostream> error_callback;



public:
	/// @name Constructors & destructors
	/// @{

	/// Not default-constructable
	Logger(void) = delete;

	/// Not copyable
	Logger(Logger&) = delete;

	/// Not movable
	Logger(Logger&&) = delete;

	/**
	 * @brief Construct a logger bound to the given file
	 *
	 * Creates a new logger connected to the specified file.
	 * 
	 * @param path A path to the file where logs are to be written.
	 * @param status_code An integer indicating the error that has occured during
	 *                    the logger initialisation.
	 * 
	 * @note After the execution of this constructor \c status_code is one of the
	 * following:
	 * * \c FASOLVER_LOGGER_STATUS_OK - if the logger was created without any errors.
	 * * \c FASOLVER_LOGGER_STATUS_BAD_PARENT - if the parent folder for the log file
	 *   does not exist and its creation was aborted by the file system.
	 * * \c FASOLVER_LOGGER_STATUS_BAD_FILENAME - if the filename has no format
	 *   specified.
	 * * \c FASOLVER_LOGGER_STATUS_BAD_FILE - if the logger was not able to open the
	 *   log file.
	 */
	explicit
	Logger(std::string const path, int *status_code) noexcept;

	/**
	 * @brief Construct a logger bound to the given stream
	 *
	 * Creates a new logger connected to the specified output stream.
	 * 
	 * @param output_stream A UTF-8-compatible output stream.
	 * @param status_code An integer indicating the error that has occured during
	 *                    the logger initialisation.
	 * 
	 * @note After the execution of this constructor \c status_code is one of the
	 * following:
	 * * \c FASOLVER_LOGGER_STATUS_OK - if the logger was created without any errors.
	 * * \c FASOLVER_LOGGER_STATUS_BAD_PARENT - if the parent folder for the log file
	 *   does not exist and its creation was aborted by the file system.
	 * * \c FASOLVER_LOGGER_STATUS_BAD_FILENAME - if the filename has no format
	 *   specified.
	 * * \c FASOLVER_LOGGER_STATUS_BAD_FILE - if the logger was not able to open the
	 *   log file.
	 */
	explicit
	Logger(std::basic_ostream<char8_t> &output_stream, int *status_code) noexcept;

	/**
	 * @brief Destroy the logger
	 *
	 * Properly destroys the logger object.
	 */
	~Logger(void);

	/// @}



	/// @name I/O
	/// @{

	/**
	 * @brief Print informational message into the log
	 *
	 * Prints message with label "INFO".
	 * 
	 * @param message A message to print.
	 * 
	 * @note This function can be used as a callback for logging of other functions.
	 */
	void info(std::string const message) const;

	/**
	 * @brief Print warning message into the log
	 *
	 * Prints message with label "WARNING".
	 * 
	 * @param message A message to print.
	 * 
	 * @note This function can be used as a callback for logging of other functions.
	 */
	void warning(std::string const message) const;

	/**
	 * @brief Print error message into the log
	 *
	 * Prints message with label "ERROR".
	 * 
	 * @param message A message to print.
	 * 
	 * @note This function can be used as a callback for logging of other functions.
	 */
	void error(std::string const message) const;

	/**
	 * @brief Retrieve a reference for an informational callback stream
	 *
	 * Retrieves a regular output stream where other function can write their
	 * information into. These messages will be printed with label "INFO".
	 * 
	 * @note Some libraries (like CPLEX) do not allow to explicitly specify
	 * callback functions for their logs. Instead, they only accept output streams
	 * where they redirect their messages into. Use this function to retrieve an
	 * informational callback handle for the logger.
	 */
	std::ostream& getInfoCallback(void) const;

	/**
	 * @brief Retrieve a reference for a warning callback stream
	 *
	 * Retrieves a regular output stream where other function can write their
	 * warnings into. These messages will be printed with label "WARNING".
	 * 
	 * @note Some libraries (like CPLEX) do not allow to explicitly specify
	 * callback functions for their logs. Instead, they only accept output streams
	 * where they redirect their messages into. Use this function to retrieve a
	 * warning callback handle for the logger.
	 */
	std::ostream& getWarningCallback(void) const;

	/**
	 * @brief Retrieve a reference for an error callback stream
	 *
	 * Retrieves a regular output stream where other function can write their
	 * warnings into. These messages will be printed with label "ERROR".
	 * 
	 * @note Some libraries (like CPLEX) do not allow to explicitly specify
	 * callback functions for their logs. Instead, they only accept output streams
	 * where they redirect their messages into. Use this function to retrieve an
	 * error callback handle for the logger.
	 */
	std::ostream& getErrorCallback(void) const;

	/// @}
};





/**
 * @class Logger::LoggerStreamBuf
 * @brief Log callback stream buffer
 * 
 * @note This is a nested class, it is not intended to be exposed to the rest of the
 * program.
 */
class Logger::LoggerStreamBuf
	: public std::streambuf
{
	/// A pointer to the enclosing object
	Logger* const logger;
	/// A pointer to the enclosing object's callback function
	void (Logger::*callback_function)(std::string const) const;
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
	 * 
	 * @param logger A pointer to the enclosing Logger object.
	 * @param callback_function A function of Logger to call when the buffer is
	 *                          synchronised.
	 * 
	 * @note \c callback_function is expected to be either Logger::info,
	 * Logger::warning, or Logger::error.
	 */
	LoggerStreamBuf(Logger* const logger, void (Logger::*callback_function)(std::string const) const);

	/// @}
};





/**
 * @class Logger::LoggerOStream
 * @brief Log callback stream
 * 
 * @note This is a nested class, it is not intended to be exposed to the rest of the
 * program.
 */
class Logger::LoggerOStream final
	: public std::ostream
	, private virtual Logger::LoggerStreamBuf
{
public:
	/// @name Constructors & destructors
	/// @{

	/**
	 * @brief Constructor
	 *
	 * Constructs a new log callback stream.
	 * 
	 * @param logger A pointer to the enclosing Logger object.
	 * @param callback_function A function of Logger to call when the buffer is
	 *                          synchronised.
	 * 
	 * @note \c callback_function is expected to be either Logger::info,
	 * Logger::warning, or Logger::error.
	 */
	LoggerOStream(Logger* const logger, void (Logger::*callback_function)(std::string const) const);

	/// @}
};





/**
 * @class Logger::LoggerCore
 * @brief Core interface of the logger
 *
 * It is used from within the Logger object to output logs into either console or
 * a regular file using the unified abstracted set of methods.
 * 
 * @note This is a nested class, it is not intended to be exposed to the rest of the
 * program.
 */
class Logger::LoggerCore final
{
	/// The output stream
	std::basic_ostream<char8_t>& output_stream;



public:
	/// @name Constructors & destructors
	/// @{

	/// Not default-constructable
	LoggerCore(void) = delete;

	/**
	 * @brief Construct a logger core
	 *
	 * Creates a core logger interface for the given output stream.
	 * 
	 * @param output_stream A basic output stream for which a unified interface
	 *                      can be used.
	 */
	LoggerCore(std::basic_ostream<char8_t>& output_stream);
	
	/// @}



	/// @name I/O
	/// @{

	/**
	 * @brief Print informational message into the log
	 *
	 * Prints message with label "INFO".
	 * 
	 * @param message A message to print.
	 */
	void info(std::string const message) const;

	/**
	 * @brief Print warning message into the log
	 *
	 * Prints message with label "WARNING".
	 * 
	 * @param message A message to print.
	 */
	void warning(std::string const message) const;

	/**
	 * @brief Print error message into the log
	 *
	 * Prints message with label "ERROR".
	 * 
	 * @param message A message to print.
	 */
	void error(std::string const message) const;

	/// @}
};





#endif // __FASOLVER_LOGGER_HPP__

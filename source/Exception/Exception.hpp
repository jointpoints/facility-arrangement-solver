/**
 * @file Exception.hpp
 * @author Andrew Eliseev (JointPoints), 2022, github.com/jointpoints
 */
#ifndef __FASOLVER_EXCEPTION_HPP__
#define __FASOLVER_EXCEPTION_HPP__





#include <string>

#define DERIVE_EXCEPTION(name)                                   \
struct name : public BaseException                               \
{                                                                \
    explicit inline                                              \
    name(std::string const message) : BaseException(message) {}; \
                                                                 \
    virtual inline                                               \
    char const *const what(void) {return this->message.data();}  \
}





class BaseException
{
protected:
	/// Message to throw
	std::string const message;



public:
	/// @name Constructors & destructors
	/// @{

	explicit inline
	BaseException(std::string const message) : message(message) {};

	/// @}



	/// @name Access
	/// @{

	virtual inline
	char const *const what(void) = 0;

	/// @}
};





DERIVE_EXCEPTION(PrerequisitesFailed);





#endif // __FASOLVER_EXCEPTION_HPP__

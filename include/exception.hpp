#pragma once
#include <exception>
#include <stdint.h>

class BusinessException : std::exception
{
    const char *message;

public:
    BusinessException(const char*);
    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;
    ~BusinessException() override = default;
};
BusinessException::BusinessException(const char* msg){
    this->message = msg;
}

const char* BusinessException::what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW{
    return this->message;
}


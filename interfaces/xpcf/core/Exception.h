#ifndef ORG_BCOM_XPCF_EXCEPTION_H
#define ORG_BCOM_XPCF_EXCEPTION_H

#include "xpcf/core/uuid.h"
#include "xpcf/xpcf_api_define.h"
#include "xpcf/core/XPCFErrorCode.h"

#include <exception>
#include <string>
namespace org { namespace bcom { namespace xpcf {

// the base xpcf exception can serve as a base that holds also an error code.
// It allows to create a Facade interface that return an error code from an underlying exception (for example a C ABI facade).
class XPCF_EXPORT_API Exception : public std::runtime_error {
public:
    Exception(XPCFErrorCode errCode);
    Exception(const char * what, XPCFErrorCode errCode = XPCFErrorCode::_FAIL);
    Exception(const std::string & what, XPCFErrorCode errCode = XPCFErrorCode::_FAIL);
    virtual ~Exception();

    inline XPCFErrorCode getErrorCode() const {
        return m_errCode;
    }

private:
    XPCFErrorCode m_errCode;
};

class XPCF_EXPORT_API AccessDeniedException : public Exception {
public:
    AccessDeniedException();
    AccessDeniedException(const char * what);
    AccessDeniedException(const std::string & what);
    virtual ~AccessDeniedException() = default;
};

class XPCF_EXPORT_API IllegalStateException : public Exception {
public:
    IllegalStateException();
    IllegalStateException(const char * what);
    IllegalStateException(const std::string & what);
    virtual ~IllegalStateException() = default;
};

// TODO : provide modulemetadata to exception ???
class XPCF_EXPORT_API ModuleException : public Exception {
public:
    ModuleException();
    ModuleException(const char * what, XPCFErrorCode errCode = XPCFErrorCode::_FAIL);
    ModuleException(const std::string & what, XPCFErrorCode errCode = XPCFErrorCode::_FAIL);
    virtual ~ModuleException() = default;
};

class XPCF_EXPORT_API NotImplementedException : public Exception {
public:
    NotImplementedException();
    NotImplementedException(const char * what);
    NotImplementedException(const std::string & what);
    virtual ~NotImplementedException() = default;
};

class XPCF_EXPORT_API NullPointerException : public Exception {
public:
    NullPointerException();
    NullPointerException(const char * what);
    NullPointerException(const std::string & what);
    virtual ~NullPointerException() = default;
};

class XPCF_EXPORT_API TimeoutException : public Exception {
public:
    TimeoutException();
    TimeoutException(const char * what);
    TimeoutException(const std::string & what);
    virtual ~TimeoutException() = default;
};


class XPCF_EXPORT_API UUIDNotFoundException : public Exception {
protected:
    UUIDNotFoundException(const uuids::uuid & uuid);
    UUIDNotFoundException(const char * what, XPCFErrorCode errCode = XPCFErrorCode::_FAIL);
    UUIDNotFoundException(const std::string & what, XPCFErrorCode errCode = XPCFErrorCode::_FAIL);
    virtual ~UUIDNotFoundException() = default;
};

class XPCF_EXPORT_API ComponentNotFoundException : public UUIDNotFoundException {
public:
    ComponentNotFoundException(const uuids::uuid & componentUUID);
    ComponentNotFoundException(const char * what);
    ComponentNotFoundException(const std::string & what);
    virtual ~ComponentNotFoundException() = default;
};

class XPCF_EXPORT_API InterfaceNotImplementedException : public Exception {
public:
    InterfaceNotImplementedException(const uuids::uuid & componentUUID, const uuids::uuid & interfaceUUID);
    InterfaceNotImplementedException(const char * what);
    InterfaceNotImplementedException(const std::string & what);
    virtual ~InterfaceNotImplementedException() = default;
};

class XPCF_EXPORT_API ModuleNotFoundException : public UUIDNotFoundException {
public:
    ModuleNotFoundException(const uuids::uuid & moduleUUID);
    ModuleNotFoundException(const char * what);
    ModuleNotFoundException(const std::string & what);
    virtual ~ModuleNotFoundException() = default;
};

}}}

#endif
